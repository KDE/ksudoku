/* This file is part of the ksudoku project
   Copyright (C) 2007 Johannes Bergmeier <Johannes.Bergmeier@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "solver.h"
		
#include <qbitarray.h>
#include "grouplookup.h"


namespace ksudoku {

class SolverState {
public:
	SolverState(int size, int order)
		: m_size(size), m_order(order), m_values(size, 0), m_flags(order),
		  m_remaining(size, order+1, order)
	{
		for(int i = order-1; i >= 0; --i) {
			m_flags[i].detach();
			m_flags[i].fill(true, size);
		}
	}
	
	SolverState(const SolverState& state)
		: m_size(state.m_size), m_order(state.m_order), m_values(state.m_values),
		  m_flags(state.m_flags), m_remaining(state.m_remaining)
	{
		for(int i = m_order-1; i >= 0; --i) {
			m_flags[i].detach();
		}
	}
	
	int value(int index) const { return m_values[index]; }
	
	inline ProcessState setValue(int index, int value, SKGraph* graph) {
		if(m_values[index] != 0) return KSS_CRITICAL;
		m_remaining.setValue(index, 0);
		m_values[index] = value;
		for(int i = 0; i < graph->optimized_d[index]; ++i) {
			int j = graph->optimized[index][i];
			if(m_values[j] == 0) {
				if(m_flags[value-1][j]) {
					m_flags[value-1].clearBit(j);
// 					if(m_remaining.value(j) == 8) printf("Aha\n");
					int remaining = m_remaining.value(j);
					if(remaining == 1) return KSS_FAILURE;
					m_remaining.setValue(j, remaining-1);
				}
			}
		}
		return KSS_SUCCESS;
	}
	
	inline ProcessState fill(const QVector<int>& values, SKGraph* graph) {
		for(int i = 0; i < m_size; ++i) {
			if(values[i] == 0) continue;
			if(setValue(i, values[i], graph) != KSS_SUCCESS)
				return KSS_FAILURE;
		}
		return KSS_SUCCESS;
	}
	
	/**
	 * Sets all values for which only one flag is left
	 * Returns wheter it failed due to conflicts.
	 */
	ProcessState setAllDefindedValues(SKGraph* graph) {
		int index;
		ProcessState state;
		while((index = m_remaining.firstIndexWithValue(1)) >= 0) {
			for(int i = 0; ; ++i) {
				// Check whethere there wasn't a flag left
				if(i >= m_order) return KSS_CRITICAL;
				
				if(m_flags[i][index]) {
					if((state = setValue(index, i+1, graph)) != KSS_SUCCESS) return state;
					break;
				}
			}
		}
		return KSS_SUCCESS;
	}
	
	int optimalSolvingIndex() {
		for(int i = 2; i <= m_order; ++i) {
			if(m_remaining.firstIndexWithValue(i) >= 0)
				return m_remaining.firstIndexWithValue(i);
		}
		return -1;
	}
	
	int possibleValue(int index, int startValue = 0) {
		if(m_values[index] != 0) return 0;
		for(int i = startValue ? startValue-1 : 0; i < m_order; ++i) {
			if(m_flags[i][index])
				return i+1;
		}
		return 0;
	}
	
private:
	int                      m_size;
	int                      m_order;
	QVector<int>       m_values;
	QVector<QBitArray> m_flags; // I don't know whether this is fast enough
	GroupLookup              m_remaining;
};


Solver::Solver(Graph* graph, uint flags) : m_graph(graph), m_flags(flags) {
	
}

Solver::~Solver() {
}

int Solver::getSymmetricIndices(int index, int out[4])
{
	int which = 0; // TODO replace this with another flag
	
	out[0] = index;
	switch(m_flags & KSS_SYM_MASK) {
		case KSS_SYM_NONE:
			return 1;
		case KSS_SYM_DIAGONAL:
			if(which == 1)
				index = (m_graph->order -index/m_graph->order -1) * m_graph->order +
				        m_graph->order -index%m_graph->order -1;
			out[1] = (index % m_graph->order) * m_graph->order + index / m_graph->order;
			return 2 - ((out[1]==out[0]) ? 1 : 0);
		case KSS_SYM_CENTRAL:
			out[1] = m_graph->size -index -1;
			return 2 - ((out[1]==out[0]) ? 1 : 0);
		case KSS_SYM_FOURWAY:
			bool b[3] = {1,1,1};
			out[1] = out[2] = out[3] = 0;
			if(m_graph->order & 0x1 == 1) {
				if((index % m_graph->order) == (m_graph->order-1)/2) b[0] = b[2] = 0;
				if((index / m_graph->order) == (m_graph->order-1)/2) b[1] = b[2] = 0;
			}
			
			int c = 1;
			if(b[2] == 0) {
				out[1] = (m_graph->order-1-index/m_graph->order)*m_graph->order +
				         m_graph->order-1-index%m_graph->order;
				if(out[1] != out[0]) c++;
			} else {
				out[1] = (m_graph->order-1-index/m_graph->order)*m_graph->order +
				         m_graph->order-1-index%m_graph->order;
				out[2] = (index/m_graph->order)*m_graph->order +
				         m_graph->order-1-index%m_graph->order;
				out[3] = (m_graph->order-1-index/m_graph->order)*m_graph->order +
				         index%m_graph->order;
				c = 4;
			}
			return c;
	}
	return 1;
}

int Solver::solve(const QVector<int>& puzzle, int maxSolutions) {
	// I got constant values in this method by trial and error
	
	SolverState state(m_graph->size, m_graph->order);
	state.fill(puzzle, m_graph);
	
	ProcessState result;
	
	// Do 20 tries to solve the puzzle, this should be enough in most cases
	for(int i = 0; i < 20; ++i) {
		// TODO This might change whith an evolved internal solver algorithmn
		
		// If no solutions were found after size*8 forks, than there 
		// will probably be no solution in a near range, and a restart of the
		// solving will give the solutions faster.
		// After the first solution was found the next solutions
		// are within few forks.
		m_forksLeft = m_graph->size * 8;
	
		m_solutionsLeft = maxSolutions;
		
		result = solveByForks(state);
		if(result != KSS_ENOUGH_FORKS) break;
	}
	
	switch(result) {
		case KSS_ENOUGH_SOLUTIONS:
			return maxSolutions;
		case KSS_SUCCESS:
			return maxSolutions - m_solutionsLeft;
		case KSS_ENOUGH_FORKS:
		case KSS_FAILURE:
			return -1;
		case KSS_CRITICAL:
		default:
			return -2;
	}
}

ProcessState Solver::solveByLastFlag(SolverState& state) {
	ProcessState ret;
	if((ret = state.setAllDefindedValues(m_graph)) != KSS_SUCCESS) return ret;
	
	int index = state.optimalSolvingIndex();
	if(index < 0) {
		m_result.resize(m_graph->size);
		for(int i = 0; i < m_graph->size; ++i)
			m_result[i] = state.value(i);
		
		if(m_solutionsLeft-- <= 1)
			return KSS_ENOUGH_SOLUTIONS;
	}
	return KSS_SUCCESS;
}

ProcessState Solver::solveByForks(SolverState& state) {
	ProcessState ret;
	if((ret = state.setAllDefindedValues(m_graph)) != KSS_SUCCESS) return ret;
	
	int index = state.optimalSolvingIndex();
	// Are there no more free fields?
	if(index < 0) {
		m_result.resize(m_graph->size);
		for(int i = 0; i < static_cast<int>(m_graph->size); ++i)
			m_result[i] = state.value(i);
// 		if(puzzle) {
// 			for(uint i = 0; i < static_cast<uint>(size); ++i) {
// 				puzzle->numbers[i] = state.value(i);
// 			}
// 		}
// 		printf("Got Solution %d\n", *solutionsLeft);
		// if we have enough solutions end searching for other solutions
		// this code would secure against preset *solutionsLeft == 0.
		if(m_solutionsLeft-- <= 1)
			return KSS_ENOUGH_SOLUTIONS;
		return KSS_SUCCESS;
	}
	
	ret = solveByLastFlag(state);
	if(ret != KSS_SUCCESS) return ret;
	
	
// 	uint startValue = RANDOM(m_graph->order);
	int startValue = rand() % m_graph->order;
	bool restart = false;
	int value = state.possibleValue(index, startValue);
	if(!value) {
		restart = true;
		value = state.possibleValue(index, 0);
	}
	// Reached a fork
	while(value) {
		// Takes the next path
		SolverState localState(state);
		
		if(m_forksLeft-- == 0)
			return KSS_ENOUGH_FORKS;
		
		// Setup the path
// 		printf("Set Cell %d to Value %d\n", index, value);
		if((ret = localState.setValue(index, value, m_graph)) != KSS_SUCCESS) return ret;
		
		// Process the path
		ret = solveByForks(localState);
		if(ret != KSS_SUCCESS && ret != KSS_FAILURE) return ret;
		
		value = state.possibleValue(index, value+1);
		if(!value && !restart) {
			restart = true;
			value = state.possibleValue(index, 0);
		}
		if(restart && value >= startValue) return KSS_SUCCESS;
	}
	
	// This path finished
	return KSS_SUCCESS;
}


}
