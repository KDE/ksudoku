/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "sudoku_solver.h"

#include <stdio.h>
#include <cstdlib>

#include <time.h>


// SUDOKU PUZZLE SOLVER BASIC ALGORITHM - BY FRANCESCO ROSSI 2005 redsh@email.it
SKPuzzle stack[625+1];

// #include <qtextstream.h>
#include <qbitarray.h>

#ifdef DEBUG
	#include <iostream>
#endif

#include "grouplookup.h"
#include "solver.h"

using namespace ksudoku;

class SolverState {
public:
	SolverState(uint size, uint order)
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
	
	uint value(uint index) const { return m_values[index]; }
	
	inline ProcessState setValue(uint index, uint value, skGraph* graph) {
		if(m_values[index] != 0) return KSS_CRITICAL;
		m_remaining.setValue(index, 0);
		m_values[index] = value;
		for(uint i = 0; i < (uint)graph->optimized_d[index]; ++i) {
			uint j = graph->optimized[index][i];
			if(m_values[j] == 0) {
				if(m_flags[value-1][j]) {
					m_flags[value-1].clearBit(j);
// 					if(m_remaining.value(j) == 8) printf("Aha\n");
					uint remaining = m_remaining.value(j);
					if(remaining == 1) return KSS_FAILURE;
					m_remaining.setValue(j, remaining-1);
				}
			}
		}
		return KSS_SUCCESS;
	}
	
	/**
	 * Sets all values for which only one flag is left
	 * Returns wheter it failed due to conflicts.
	 */
	ProcessState setAllDefindedValues(skGraph* graph) {
		int index;
		ProcessState state;
		while((index = m_remaining.firstIndexWithValue(1)) >= 0) {
			for(uint i = 0; ; ++i) {
				// Chekc whetere there wasn't a flag left
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
		for(uint i = 2; i <= m_order; ++i) {
			if(m_remaining.firstIndexWithValue(i) >= 0)
				return m_remaining.firstIndexWithValue(i);
		}
		return -1;
	}
	
	uint possibleValue(uint index, uint startValue = 0) {
		if(m_values[index] != 0) return 0;
		for(uint i = startValue ? startValue-1 : 0; i < m_order; ++i) {
			if(m_flags[i][index])
				return i+1;
		}
		return 0;
	}
	
private:
	uint               m_size;
	uint               m_order;
	QVector<uint>      m_values;
	QVector<QBitArray> m_flags; // I don't know whether this is fast enough
	GroupLookup        m_remaining;
};


//
// class SKSolver
//

SKSolver::SKSolver(SKGraph* gr)
{
	base = gr->base;
	order = gr->order;
	g = gr;
	m_type = sudoku;
	size = gr->size;
}

SKSolver::SKSolver(int n, bool threedimensionalf)
{
	base = static_cast<int>(sqrt(n));
	order=n;
	m_type = threedimensionalf ? roxdoku : sudoku;
	           ///@TODO fix above so more than 2
	           ///      game types can be supported

	if(threedimensionalf)
		size = base*order;
	else
		size = n*n;
}

SKSolver::~SKSolver()
{
	delete g;
}

int SKSolver::get_simmetric(int order, int size, int type, int idx, int which, int out[4])
{
	out[0] = idx;
	switch (type)
		{
			case SIMMETRY_NONE:
				return 1;
			case SIMMETRY_DIAGONAL:
				if(which == 1)
					idx = (order-1-idx/order)*order+order-1-idx%order;
				out[1] = (idx%order)*order+idx/order;
				return 2-(out[1]==out[0]);
			case SIMMETRY_CENTRAL:
				out[1] =size-idx-1;
				return 2-(out[1]==out[0]);
			case SIMMETRY_FOURWAY:
				bool b[3] = {1,1,1};
				out[1]=out[2]=out[3]=0;
				if(order % 2 == 1)
				{
					if((idx % order) == (order-1)/2) b[0] = b[2]=0;
					if((idx / order) == (order-1)/2) b[1] = b[2]= 0;
				}

				int c=1;

				if(b[2]==0)
				{
					out[1] = (order-1-idx/order)*order+order-1-idx%order;
						if(out[1] != out[0]) c++;
				}
				else {
					out[1] = (order-1-idx/order)*order+order-1-idx%order;
					out[2] = (idx/order)*order+order-1-idx%order;
					out[3] = (order-1-idx/order)*order+idx%order;
					c =4;
				}
				
				/*printf("%d (%d %d) (%d %d) (%d %d) (%d %d)  - %d %d %d\n", c, idx%order, idx/order, 
																		out[1]%order, out[1]/order, 
																		out[2]%order, out[2]/order, out[3]%order, out[3]/order, b[0],  b[1], b[2]);*/
				return c;
		}
	return 1;
}

uint SKSolver::getSymmetry(uint flags, int index, int out[4])
{
	return ksudoku::Solver(g, flags).getSymmetricIndices(index, out);
// 	int which = 0; // TODO replace this with another flag
// 	
// 	out[0] = index;
// 	switch(flags & KSS_SYM_MASK) {
// 		case KSS_SYM_NONE:
// // 			printf("Sym: None\n");
// 			return 1;
// 		case KSS_SYM_DIAGONAL:
// // 			printf("Sym: Diagonal\n");
// 			if(which == 1)
// 				index = (order-1-index/order)*order+order-1-index%order;
// 			out[1] = (index%order)*order+index/order;
// 			return 2 - ((out[1]==out[0])?1:0);
// 		case KSS_SYM_CENTRAL:
// // 			printf("Sym: Central\n");
// 			out[1] = size-index-1;
// // 			printf("%d == %d", out[0], out[1]);
// 			return 2 - ((out[1]==out[0])?1:0);
// 		case KSS_SYM_FOURWAY:
// // 			printf("Sym: Fourway\n");
// 			bool b[3] = {1,1,1};
// 			out[1] = out[2] = out[3] = 0;
// 			if(order & 0x1 == 1) {
// 				if((index % order) == (order-1)/2) b[0] = b[2] = 0;
// 				if((index / order) == (order-1)/2) b[1] = b[2] = 0;
// 			}
// 			
// 			int c = 1;
// 			if(b[2] == 0) {
// 				out[1] = (order-1-index/order)*order+order-1-index%order;
// 				if(out[1] != out[0]) c++;
// 			} else {
// 				out[1] = (order-1-index/order)*order + order-1-index%order;
// 				out[2] = (index/order)*order+order-1-index%order;
// 				out[3] = (order-1-index/order)*order+index%order;
// 				c = 4;
// 			}
// 			return c;
// 	}
// 	
// // 	printf("Sym: WTF?\n");
// 	return 1;
}

int SKSolver::remove_numbers(SKPuzzle* p, int level, int simmetry, int type)
{
	//if(type!=2) //not custom
	//	return remove_numbers2(p, level, simmetry, type);
			
//	std::srand(time(0));
//	int  b;
	int cnt=p->size;
	int to=p->size*(4+6*(level==-1));
	int solutions_d=0;

	//SKPuzzle stack[257];
	bool done[625+1];
	ITERATE(i,p->size+1){ stack[i].setorder(order); done[i]=false;};
		
	if(p->size>81) to = (p->size-32);
	if(p->size>441) to = (p->size-32)/2;
	//if(p->order>16) to = (p->size-32)/(2);
	SKPuzzle c(p->order);
	copy(&c, p);
	
	if(level < 0)
	{
		//TOO BUGGY
		//int val = RANDOM(p->order)+1;
		//ITERATE(i,size)
		//	if(p->numbers[i] == val){ p->numbers[i] = 0;  p->numbers[p->size-i-1]=0; cnt-=2;}
		simmetry = SIMMETRY_NONE;
	}

	if(simmetry == SIMMETRY_RANDOM) simmetry = RANDOM(3)+2;
	int which = RANDOM(2);
	//if(type == 1) simmetry = SIMMETRY_CENTRAL;
	if(simmetry == SIMMETRY_FOURWAY && order == 16) to=to*1/2;
	if(order == 25) simmetry = SIMMETRY_NONE; // TOO SLOW

	ITERATE(q, to) 
	{
		int idx = RANDOM(p->size);  //2FIX
//		printf("%d/%d\n", q,to);
		int index [4]; //{idx, };
		int index_d = get_simmetric(order, size,simmetry, idx, which, index);

		bool go=true;
		ITERATE(i, index_d) if(g->optimized_d[index[i]]==0) 
			go=false;
		if(!go) 
		{
			//printf("unlinked node %d %d\n",index_d,simmetry); 
			q--; 
			continue;
		}
		
		int backup[4] = {0,0,0,0};
		
		int n_err = 0;
		ITERATE(i, index_d)
		{
			backup[i] = p->numbers[index[i]];
			if(backup[i] != 0 && done[index[i]] == false) 
			{
				done[index[i]] = true;
				solutions_d=0;
				p->numbers[index[i]] = 0;
				copy(&stack[0], p);
				solve_engine(&stack[0], solutions_d, 0, 2, index[i], index[i], backup[i]);
				n_err += (solutions_d != 1);
			}
			else
				n_err=1;
		}
		if(n_err > 0)
			ITERATE(i, index_d)
				p->numbers[index[i]] = backup[i];
		else	
			cnt-=index_d;
	}
	
	
	int numberOfNumbersToAdd = (7*level*(((type!=1) ? ((int) sqrt(p->size)) : p->order )+LEVINC-(p->order-2)*(type==1)))/10;
	printf("%d\n", numberOfNumbersToAdd);

	ITERATE(i, numberOfNumbersToAdd)
	{
		int idx = RANDOM(p->size);//2FIX
		int orig = idx;
		while(p->numbers[idx] != 0) 
		{
			idx=(idx+1) % p->size;
			if(idx==orig) return cnt;
		}
		p->numbers[idx] = c.numbers[idx];
		int index[4];
		int index_d = get_simmetric(order, size,simmetry, idx, which, index);
		ITERATE(j, index_d)
		{
			p->numbers[index[j]] = c.numbers[index[j]];
			i++;	
			cnt++;
		}
	}
	/* GENERATES PUZZLES WITH MULTIPLE SOLUTIONS
	if(level < 0)
	{
		ITERATE(i,2)
		{
			int idx = RANDOM(p->size);//2FIX
			while(p->numbers[idx] == 0) idx=(idx+1) % p->size;
			p->numbers[idx] = 0;
			cnt--;
		}
	}*/
	
	return cnt;
}

#include <qdatetime.h> // HACK this is only required for the benchmark

int SKSolver::remove_numbers2(SKPuzzle* p, int level, int simmetry, int typeo)
{
// 	return 0;
	QVector<uint> numbers(size);
	
	for(uint i = 0; i < (uint)size; ++i)
		numbers[i] = p->numbers[i];
	
	uint flags = 0;
	
	if(typeo == 1) simmetry = SIMMETRY_CENTRAL;
	switch(simmetry) {
		case SIMMETRY_DIAGONAL:
			flags |= KSS_SYM_DIAGONAL;
			break;
		case SIMMETRY_CENTRAL:
			flags |= KSS_SYM_CENTRAL;
			break;
		case SIMMETRY_FOURWAY:
			flags |= KSS_SYM_FOURWAY;
			break;
		case SIMMETRY_NONE:
		default:
			flags |= KSS_SYM_NONE;
			break;
	}
	
	if(level == -1) flags |= KSS_REM_1VALUE;
	
	int hints = level*(order+LEVINC-(order-2)*typeo);
	removeValuesSimple(numbers, (hints>0) ? hints : 0, flags);
	
	for(uint i = 0; i < (uint)size; ++i)
		p->numbers[i] = numbers[i];
		
	return 1;
	
	
	// Solver Benchmark !!!! TODO: remove this
	printf("Do benchmark!\n");
	QTime time;
	time.start();
	for(uint i = 100; i != 0; --i) {
		solve2(p);
		printf("%d\n", i);
	}
	uint oldT = time.elapsed();
	
	time.start();
	for(uint i = 100; i != 0; --i) {
		solve(p);
		printf("%d\n", i);
	}
	uint newT = time.elapsed();
	printf("Test old: %d\n", oldT);
	printf("Test new: %d\n", newT);
	return 1;
	
}

uint SKSolver::removeValuesSimple(QVector<uint>& puzzle, uint hints, uint flags) {
	QVector<uint> local(puzzle);
	int cellsLeft = size;
	
	// completely remove all occurences of a random value
	if(flags && KSS_REM_1VALUE) {
		uint startValue = RANDOM(order)+1;
		uint i;
		for(i = startValue; i <= (uint)order; ++i) {
			uint remCount = removeValueCompletely(local, i, flags);
			if(remCount != 0) {
				cellsLeft -= remCount;
				break;
			}
		}
		if(i > (uint)order) {
			for(i = 1; i < startValue; ++i) {
				uint remCount = removeValueCompletely(local, i, flags);
				if(remCount != 0) {
					cellsLeft -= remCount;
					break;
				}
			}
			if(i == startValue)
				return 0;
		}
	}
	
	uint failures = 0;
	// scanning until order instead of base might remove about 2-5 more values
	while(failures < (uint)base) {
		uint startIndex = RANDOM(size);
		uint index = startIndex;
		do {
			if(local[index] != 0) break;
			index = (index+1)%size;
		} while(index != startIndex);
		
		uint remCount = removeAtIndex(local, index, flags);
		if(remCount != 0) {
			cellsLeft -= remCount;
			if(failures) --failures;
		} else {
			++failures;
		}
		printf("Failures: %d - %d\n", cellsLeft, failures);
	}
	
	// give initial hints
	for(uint i = hints; i != 0; --i) {
		uint startIndex = RANDOM(size);
		uint index = startIndex;
		do {
			if(local[index] == 0) {
				local[index] = puzzle[index];
				break;
			}
			index = (index+1)%size;
		} while(index != startIndex);
	}
	cellsLeft += hints;
	
	puzzle = local;
	
	return cellsLeft;
	
}

int SKSolver::removeValues(QVector<uint>& puzzle, uint count, uint flags) {
	QVector<uint> local(puzzle);
	int removesLeft = count;
	
	if(flags && KSS_REM_1VALUE) {
		uint startValue = RANDOM(order)+1;
		uint i;
		for(i = startValue; i <= (uint)order; ++i) {
			uint remCount = removeValueCompletely(local, i, flags);
			if(remCount != 0) {
				removesLeft -= remCount;
				break;
			}
		}
		if(i > (uint)order) {
			for(i = 1; i < startValue; ++i) {
				uint remCount = removeValueCompletely(local, i, flags);
				if(remCount != 0) {
					removesLeft -= remCount;
					break;
				}
			}
			if(i == startValue)
				return 0;
		}
	}
	
	while(removesLeft > 0) {
		uint startIndex = RANDOM(size);
		uint index = startIndex;
		do {
			if(local[index] != 0) {
				uint remCount = removeAtIndex(local, index, flags);
				if(remCount != 0) {
					removesLeft -= remCount;
					break;
				}
			}
			index = (index+1)%size;
		} while(index != startIndex);
	}
	
	puzzle = local;
	return 1;
}

uint SKSolver::removeValueCompletely(QVector<uint>& puzzle, uint value, uint flags) {
	QVector<uint> local(puzzle);
	uint count = 0;
	
	for(uint i = 0; i < (uint)size; ++i) {
		if(local[i] == value) {
			uint remCount = removeAtIndex(local, i, flags);
			if(remCount == 0) return 0;
			count += remCount;
		}
	}
	
	puzzle = local;
	return count;
}

uint SKSolver::removeAtIndex(QVector<uint>& puzzle, uint index, uint flags) {
	int indices[4];
	int oldValues[4];
	int count;
	
	count = getSymmetry(flags, index, indices);
	for(int i = 0; i < count; ++i) {
		oldValues[i] = puzzle[indices[i]];
		puzzle[indices[i]] = 0;
	}
	
	::SolverState state(size, order);
	for(uint i = 0; i < static_cast<uint>(size); ++i) {
		if(puzzle[i]) state.setValue(i, puzzle[i], g);
	}
	
	uint forksLeft = size * 8;
	uint solutionsLeft = 2;
	solveEngine(state, 0, &solutionsLeft, &forksLeft);
	if(solutionsLeft == 1) {
		return count;
	}
	
	for(int i = 0; i < count; ++i) {
		puzzle[indices[i]] = oldValues[i];
	}
	return 0;
}

int SKSolver:: solve(SKPuzzle* puzzle, int max_solutions, SKPuzzle* out_solutions, int* /*forks*/) {
// 	return solve2(puzzle, max_solutions, out_solutions, forks);
	
	if(puzzle->order != order) return -1;
	if(puzzle->size != size) return -1;
	if(g == 0) return -2;
	
	ksudoku::Solver mySolver(g);
	
// // 	SolverState state(puzzle->size, puzzle->order);
// 	ksudoku::SolverState state(puzzle->size, puzzle->order);
// 	
// 	for(uint i = 0; i < static_cast<uint>(size); ++i) {
// 		if(puzzle->numbers[i])
// 			state.setValue(i, puzzle->numbers[i], g);
// 	}
// 	
// 	max_solutions = 10;
// // 	
// // 	uint forksLeft = size * 8;
// // 	uint solutionsLeft = max_solutions;
// 	
// 	
// 	for(uint i = 0; i < 20; ++i) {
// // // 		printf("Try %d\n", i);
// // 		forksLeft = size * 8;
// // 		solutionsLeft = max_solutions;
// // 		
// // 		ProcessState ret = solveEngine(state, out_solutions, &solutionsLeft, &forksLeft);
// 		mySolver.m_solutionsLeft = max_solutions;
// 		mySolver.m_forksLeft = size*8;
// 		
// 		ProcessState ret = mySolver.solveByForks(state);
// 		if(ret != KSS_ENOUGH_FORKS) break;
// 	}
// 	if(forks) *forks = size*8 - mySolver.m_forksLeft;
	
	QVector<int> v(size);
	
	for(int i = 0; i < size; ++i)
		v[i] = puzzle->numbers[i];
	
	int ret = mySolver.solve(v, max_solutions);
	if(ret < 1) return -3;
	
	if(out_solutions) {
		QVector<int> values = mySolver.result();
		for(uint i = 0; i < (uint)size; ++i)
			out_solutions->numbers[i] = values[i];
	}
		
// 	
// 	return max_solutions - solutionsLeft;
// 	return max_solutions - mySolver.m_solutionsLeft;
	return ret;
	
}

ProcessState SKSolver::solveEngine(::SolverState& state, SKPuzzle* puzzle, uint* solutionsLeft, uint* forksLeft) {
	ProcessState ret;
	if((ret = state.setAllDefindedValues(g)) != KSS_SUCCESS) return ret;
	
	int index = state.optimalSolvingIndex();
	// Are there no more free fields?
	if(index < 0) {
		if(puzzle) {
			for(uint i = 0; i < static_cast<uint>(size); ++i) {
				puzzle->numbers[i] = state.value(i);
			}
		}
// 		printf("Got Solution %d\n", *solutionsLeft);
		// if we have enough solutions end searching for other solutions.
		// this code secures agains *solutionsLeft == 0.
		if((*solutionsLeft)-- <= 1)
			return KSS_ENOUGH_SOLUTIONS;
		return KSS_SUCCESS;
	}
	
	uint startValue = RANDOM(order);
	bool restart = false;
	uint value = state.possibleValue(index, startValue);
	if(!value) {
		restart = true;
		value = state.possibleValue(index, 0);
	}
	// Reached a fork
	while(value) {
		// Takes the next path
		::SolverState localState(state);
		
		if((*forksLeft)-- == 0)
			return KSS_ENOUGH_FORKS;
		
		// Setup the path
// 		printf("Set Cell %d to Value %d\n", index, value);
		if((ret = localState.setValue(index, value, g)) != KSS_SUCCESS) return ret;
		
		// Process the path
		ret = solveEngine(localState, puzzle, solutionsLeft, forksLeft);
		switch(ret) {
			case KSS_CRITICAL:
				return KSS_CRITICAL;
			case KSS_ENOUGH_SOLUTIONS:
				return KSS_ENOUGH_SOLUTIONS;
			case KSS_ENOUGH_FORKS:
				return KSS_ENOUGH_FORKS;
			case KSS_SUCCESS:
			case KSS_FAILURE:
				break;
		}
		
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

SKPuzzle* head;


#define MAX_FORKS 15000
int SKSolver:: solve2(SKPuzzle* puzzle, int max_solutions, SKPuzzle* out_solutions, int* forks)
{
	if(puzzle->order != order) return -1;
	if(g == 0) return -2;
	int solutions_d=0;
	
	int ffs=0;
	if(!forks) forks = &ffs;

// 	if(puzzle->order == 25 && puzzle->threedimensional==0)
// 		forks = &ffs;
	
	head = &stack[0];
	ITERATE(i,puzzle->size+1)
	{
		stack[i].setorder(order, puzzle->type);
		stack[i].size = puzzle->size;
	}
	ITERATE(i,puzzle->size)
		if(g->optimized_d[i]==0) 
			puzzle->numbers[i]=1;

	copy(&stack[0], puzzle);
	solve_engine(&stack[0], solutions_d, out_solutions, max_solutions, -1, -1, 0, forks);
	if(puzzle->order == 25 && puzzle->type==0 && *forks > MAX_FORKS)
	{
		if(max_solutions <= 1) return -3;
		
		ITERATE(i,puzzle->size) puzzle->numbers[i] = 0;
		solve(puzzle, 1, puzzle,0);
	}
	if(forks) printf("%d\n", *forks);
	
	return solutions_d;
}

int SKSolver:: solve_engine(SKPuzzle *s,  int& solutions, SKPuzzle* solution_list, int maxsolutions, int last_add,	 int dynindex, int dynvalue, int* forks) //last_add for further optimizations
{
	if(forks && s->order == 25 && s->type==0)
	{	
		//printf("%d\n", *forks);
		if((*forks) > MAX_FORKS)
			return -1;
	}
	
	if(maxsolutions>0 && solutions>=maxsolutions)	return 0;
	
	if(dynindex!=-1)if(dynvalue == s->numbers[dynindex])
	{
		solutions++;
		return 1;
	}
	/*{//DEBUG 
		printf("%d Solution found TC\n", solutions);
		ITERATE(i,s->size)
		{
			printf("%c", zerochar  + s->numbers[i]);
			if(i%s->order == s->order-1) printf("\n");
		}
	}*/
	//int rr = 0;
	int lowest_pos,lowest,lowest_val;
	lowest_pos = 0;
	lowest_val = 0;

	lowest = s->order+1;
		
	for(int i=last_add*(last_add != -1); i<(last_add+1)+s->size*(last_add == -1); i++)
		if(s->numbers[i] != 0)
			ITERATE(j,g->optimized_d[i])
				if(s->numbers[g->optimized[i][j]] == 0)
					s->flags[g->optimized[i][j]][s->numbers[i]] = 0;
		
	for(int q=0; (last_add==-1) ? q<s->size : q<g->optimized_d[last_add]; q++)
	{
		int i = (last_add==-1) ? q : g->optimized[last_add][q];
		if(s->numbers[i] == 0)
		{
			int c=0;
			ITERATE(j,s->order)
				c+=s->flags[i][j+1];
				
			/*if(c==lowest) //otherwise i got problems with order = 25
			{
				if(RANDOM(++rr) == 0);
				lowest--;
			}*/
			if(c<lowest)
			{
				lowest_pos = i;
				lowest     = c;

				if(lowest < 1)
					return -1;
				if(lowest == 1)
				{
					ITERATE(j,s->order) if(s->flags[lowest_pos][j+1] == 1) lowest_val = j+1;
					//ITERATE(j, s-head) printf(" "); printf("ADDED %d %d\n", lowest_pos, lowest_val);
					s->numbers[lowest_pos] = lowest_val;

					return solve_engine(s, solutions, solution_list, maxsolutions, lowest_pos,-1,0,forks);
				}
			}
		}
	}
	
	if(last_add != -1) return solve_engine(s, solutions, solution_list, maxsolutions, -1,-1,0,forks);
	//check completed
	int remaining=0;
	ITERATE(i,s->size)if(s->numbers[i] == 0)remaining++;

	if(remaining == 0)
	{
	
		if(solution_list) 
			if(&solution_list[solutions]) 
				copy(&solution_list[solutions], s);
		solutions++;
	
		/*{
			printf("%d Solution found TC\n", solutions);
			ITERATE(i,s->size)
			{
				printf("%c", zerochar  + s->numbers[i]);
				if(i%s->order == s->order-1) printf("\n");
			}
		}*/
		return 1;
	}
	
	if(remaining == 1) return -1;
	
	//fork on lowest if not added
	int positions[26]; //2fix
	int positions_d = 0;
	
	ITERATE(i, s->order)
		if(s->flags[lowest_pos][i+1])
			positions[positions_d++] = i+1;
			
	while(positions_d>0)
	{
		copy(&s[1], s);
		int index = RANDOM(positions_d);
		s[1].numbers[lowest_pos] = positions[index];
		solve_engine(&s[1], solutions, solution_list, maxsolutions, lowest_pos, -1, 0, forks);

		if(forks) (*forks)++;
		for(int i=index; i<positions_d-1; i++) positions[i] = positions[i+1];
		positions_d--;
	}
	
	return -1;	
}

inline void SKSolver::addConnection(int i, int j) {
	ITERATE(k,g->optimized_d[i]) {
		if(g->optimized[i][k] == j)
			return;
	}
	g->optimized[i][g->optimized_d[i]++] = j;
}


int SKSolver:: init()
{
	if(m_type==0)
		g = new ksudoku::GraphSudoku(order);
	else if(m_type==1)
		g = new ksudoku::GraphRoxdoku(order);

	g->init();

	if(order>9) zerochar = 'a'-1;
	else zerochar = '0';
	
	//findStronglyConnectedComponents
	/*printf("Now fscc!\n");
	bool done[625+1];
	bool visited[625+1];

		ITERATE(i,125) ITERATE(j,size) g->strongly_connected[i][j] = 0;	

	g->sc_count=0;
	ITERATE(i,size)
	{
		ITERATE(j,size) visited[j]=0;
		fscc(visited, done, g->graph[i], i);
		done[i]=1;
	}
	printf("%d\n",g->sc_count);
	int* optimized_sc_d = new int[g->sc_count];
	int** optimized_sc   = new (int*)[g->sc_count];
	
	
	ITERATE(i, g->sc_count)
	{
		optimized_sc_d[i] = 0;
		ITERATE(j,size)
			if(g->strongly_connected[i][j] == 1)
				optimized_sc_d[i]++;
		optimized_sc[i] = new int[optimized_sc_d[i]];
		int c=0;
		ITERATE(j,size)
		{
			if(g->strongly_connected[i][j] == 1)
				{optimized_sc[i][c++] = j;
				printf("%d", 1);
				}else printf("%d",0);

		}
			printf("\n");
		
	}*/
		
	return 0;
}

// 	//findStronglyConnectedComponents
// int SKSolver::fscc(bool* /*visited*/, bool* /*done*/, bool /*mask*/[625], int /*node*/)
// {
// 	/*if(done[node]==1) return 0;
// 	int cc=0;
// 	ITERATE(i, size) if(mask[i] == 1) cc++;
// 	if(cc == 0) return  0;	
// 
// 	visited[node] = 1;
// 
// 	int c=0;	
// 	bool* m = new bool[size];	//2FIX
// 	//bool m[256];
// 	
// 	ITERATE(i,size)
// 		if(mask[i] == 1 && visited[i] == false)
// 		{
// 			c=1;
// 			if(done[i] == false)
// 			{
// 				ITERATE(j,size) m[j] = mask[j] & g->graph[i][j];
// 				c+=fscc(visited, done, m, i);	
// 			}
// 		}	
// 	if(c==0)
// 	{
// 		ITERATE(i, size){ g->strongly_connected[g->sc_count][i] = mask[i];}
// 		/ *ITERATE(i, g->sc_count)
// 			ITERATE(j, size)
// 				if(g->strongly_connected[i][j] != g->strongly_connected[g->sc_count][j])
// 				{
// 					g->sc_count++;
// 					visited[node] = 0;
// 					return 1;
// 				//}
// 		return 1;
// 	}
// 	//visited[node] = 0;
// 	return 0;*/
// #ifdef DEBUG
// 	std::cerr << "SKSolver::fscc NOT implemented => don't use it, this is a bug"
// 	          << std::endl;
// 	exit(1);
// 	return 0;
// #endif
// }

void SKSolver::copy(SKPuzzle* dest, SKPuzzle* src)
{
	dest->order = src->order;
	dest->base  = src->base;
	dest->size  = src->size;

	ITERATE(i, src->size)
	{
		dest->numbers[i] = src->numbers[i];
		ITERATE(j, src->order+1)
			dest->flags[i][j] = 1;//src->flags[i][j];
	}
	
}

