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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "sksolver.h"

#include <stdio.h>
// #include <cstdlib>

#include <time.h>

// SUDOKU PUZZLE SOLVER BASIC ALGORITHM - BY FRANCESCO ROSSI 2005 redsh@email.it

// #include <qtextstream.h>
#include <qbitarray.h>

#include <iostream>

#include "solver.h"

using namespace ksudoku;
using namespace std;

//
// class SKSolver
//

SKSolver::SKSolver(SKGraph* gr)
{
	g = gr;
// 	m_type = TypeSudoku;
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

int SKSolver::remove_numbers(SKPuzzle* p, int level, int symmetry, int type)
{
	SKPuzzle solution(p->order);
	copy(&solution, p);

	int cellsLeft = g->size();
	
	// Select symmetry
	int symmetryAxis = RANDOM(2);
	if(type != 0 || g->sizeZ() > 1) {
		// The dimension is to high or the board might not be symmetric
		symmetry = SIMMETRY_NONE;
	} else if(g->order() >= 16 || level > 3) {
		// Disable symmetry for performance reasons
		symmetry = SIMMETRY_NONE;
	} else {
		// Select random symmetry
		if(symmetry == SIMMETRY_RANDOM) symmetry = RANDOM(3)+2;
	}

	// Select the count of aims to remove cells
	int countOfAims = g->size() * (4 + 6 * (level > 3));
	if(p->size > 81) countOfAims = (p->size-32);
	if(p->size > 441) countOfAims = (p->size-32)/2;
	if(symmetry == SIMMETRY_FOURWAY && g->order() == 16) countOfAims = countOfAims*1/2;

	// TODO use solver from new engine directly which should vastly improve performance

	// Aims countOfAims times to remove some numbers
	for(int aim = 0; aim < countOfAims; ++aim) {
		int positions [4];
		int backup[4];
		int positionCount = 0;

		// Select the positions where to remove numbers
		for(;;) {
			// TODO this might result in an endless loop
			int idx = RANDOM(p->size);
			positionCount = get_simmetric(g->order(), g->size(), symmetry, idx, symmetryAxis, positions);
			// check whether all cells exist
			switch(positionCount) {
				case 4: if(g->optimized_d[positions[3]] == 0) continue;
				case 3: if(g->optimized_d[positions[2]] == 0) continue;
				case 2: if(g->optimized_d[positions[1]] == 0) continue;
				case 1: if(g->optimized_d[positions[0]] == 0) continue;
					break;
				default: continue;
			}
			// check whether they weren't removed yet (it's enough to test one cell)
			if(p->numbers[positions[0]] == 0) continue;
			break;
		}

		// Remove numbers
		switch(positionCount) {
			case 4: backup[3] = p->numbers[positions[3]]; p->numbers[positions[3]] = 0;
			case 3: backup[2] = p->numbers[positions[2]]; p->numbers[positions[2]] = 0;
			case 2: backup[1] = p->numbers[positions[1]]; p->numbers[positions[1]] = 0;
			case 1: backup[0] = p->numbers[positions[0]]; p->numbers[positions[0]] = 0;
		}
		
		// Validate the resulting game
		Problem problem;
		puzzleToProblem(&problem, p);
		Solver solver;
		solver.setLimit(2);
		solver.loadProblem(&problem);
		solver.solve();
		if(solver.results().size() != 1) {
			// Revert cells as they will not result in a acceptable game
			switch(positionCount) {
				case 4: p->numbers[positions[3]] = backup[3];
				case 3: p->numbers[positions[2]] = backup[2];
				case 2: p->numbers[positions[1]] = backup[1];
				case 1: p->numbers[positions[0]] = backup[0];
			}
		} else {
			// This iteration results in an acceptable game
			cellsLeft -= positionCount;
		}
	}

	// Readd some numbers
	int numberOfNumbersToAdd = (7*(3-level)*(((type!=1) ? ((int) sqrt((double)(p->size))) : p->order )+LEVINC-(p->order-2)*(type==1)))/10;
 	//printf("%d\n", numberOfNumbersToAdd);

	for(int i = 0; i < numberOfNumbersToAdd; ++i)
	{
		int idx = RANDOM(p->size);//2FIX
		int orig = idx;
		while(p->numbers[idx] != 0)
		{
			idx=(idx+1) % p->size;
			if(idx==orig) return cellsLeft;
		}
		p->numbers[idx] = solution.numbers[idx];
		int index[4];
		int index_d = get_simmetric(g->order(), g->size(), symmetry, idx, symmetryAxis, index);
		for(int j = 0; j < index_d; ++j)
		{
			p->numbers[index[j]] = solution.numbers[index[j]];
			i++;
			cellsLeft++;
		}
	}
	
	return cellsLeft;
}

#include <ruleset.h>
#include <item.h>
#include <choiceitem.h>
#include <sudokuconstraint.h>
#include <solver.h>
#include <problem.h>

int SKSolver:: solve(const Problem &problem, int max_solutions, SKPuzzle* out_solutions, int* /*forks*/) {
	Solver solver;
	solver.setLimit(max_solutions);
	solver.loadProblem(&problem);

	solver.solve();

	QVector<Problem> results = solver.results();
	
	if(results.size() < 1) return -1;

	if(out_solutions) {
		problemToPuzzle(out_solutions, &results[0]);
	}

	return results.size();
}

void SKSolver::copy(SKPuzzle* dest, SKPuzzle* src)
{
	dest->order = src->order;
	dest->size  = src->size;

	for(int i = 0; i < src->size; ++i)
	{
		dest->numbers[i] = src->numbers[i];
		for(int j = 0; j < src->order+1; ++j)
			dest->flags[i][j] = 1;//src->flags[i][j];
	}

}

bool SKSolver::puzzleToProblem(Problem* dest, SKPuzzle* source) const {
	// Create Problem
	*dest = Problem(g->m_ruleset);
	// Copy values
	for(int x = 0; x < g->sizeX(); ++x) {
		for(int y = 0; y < g->sizeY(); ++y) {
			for(int z = 0; z < g->sizeZ(); ++z) {
				int value = source->numbers[g->cellIndex(x, y, z)];
				Item *item = g->m_board->itemAt(x, y, z);
				if(item && value)
					static_cast<ChoiceItem*>(item)->setValue(dest, value);
			}
		}
	}
	return true;
}

bool SKSolver::problemToPuzzle(SKPuzzle* dest, Problem* source) const {
	for(int x = 0; x < g->sizeX(); ++x) {
		for(int y = 0; y < g->sizeY(); ++y) {
			for(int z = 0; z < g->sizeZ(); ++z) {
				Item *item = g->m_board->itemAt(x, y, z);
				if(item) {
					ChoiceItem *choiceItem = static_cast<ChoiceItem*>(item);
					dest->numbers[g->cellIndex(x, y, z)] = choiceItem->value(source);
				} else {
					dest->numbers[g->cellIndex(x, y, z)] = 1;
				}
			}
		}
	}
    return true;
}
