/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
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

#include "puzzle.h"
#include <cstdlib>
#include <time.h>
#include <qstring.h>

#include <QtDebug>
#include <choiceitem.h>

namespace ksudoku {

Puzzle::Puzzle(SKSolver* solver, bool withSolution)
	: m_withSolution(withSolution)
	, m_puzzle(0)
	, m_solution(0)
	, m_solver(solver)
	, m_difficulty(0)
	, m_symmetry(0)
{ }

Puzzle::~Puzzle() {
	delete m_puzzle;
	delete m_solution;

	m_puzzle   = 0;
	m_solution = 0;
}

bool Puzzle::init() {
	if(m_puzzle)
		return false;
	
	if(m_withSolution)
		return false;

	m_puzzle = new SKPuzzle(m_solver->g->order(), m_solver->g->oldtype, m_solver->g->size());

	for(int i = 0; i < m_puzzle->size; ++i)
		m_puzzle->numbers[i] = 0;
// 		m_puzzle->setValue(i, 0);

	return true;
}

bool Puzzle::createPartial(Solver *solver)
{
	// TODO after finding a solution try to find simpler ones
	const Ruleset *ruleset = m_solver->g->rulset();
	const ItemBoard *board = m_solver->g->board();
	for(;;) {
		ChoiceItem *choiceItem;
		for(;;) {
			int x = rand() % board->size(0);
			int y = rand() % board->size(1);
			int z = rand() % board->size(2);
			Item *item = board->itemAt(x, y, z);
			if(!item) continue;
			choiceItem = static_cast<ChoiceItem*>(item);
			if(choiceItem->value(&solver->state())) continue;
			break;
		}
		// TODO don't iterate from the minimum to the maximum
		// this will create relative easy puzzles with concentration of low values and almost no to none 9
		// TODO prepare the state to speedup deeper tries
		int min = choiceItem->minValue();
		int max = choiceItem->maxValue();
		int range = choiceItem->maxValue() - min + 1;
		int start = rand() % range;
		// count from start downward and continuing with max after min
		for(int i = range+start; i > start; --i) {
			int val = min + i % range;
			
			if(!choiceItem->marker(&solver->state(), val)) continue;

			solver->push();
			choiceItem->setValue(&solver->state(), val);
			solver->push();
			solver->solve();
			int count = solver->results().count();
			if(count == 1) 
				m_solver->problemToPuzzle(m_solution, &solver->results()[0]);
			solver->pop();
			
			if(count == 1) {
				m_solver->problemToPuzzle(m_puzzle, &solver->state());
				return true;
			} else if(count > 1) {
				if(createPartial(solver))
					return true;
			}
			
			solver->pop();
		}
	}
	return false;
}

bool Puzzle::init(int difficulty, int symmetry) {
	m_solution = new SKPuzzle(m_solver->g->order(), m_solver->g->oldtype, m_solver->g->size());
	m_puzzle = new SKPuzzle(m_solver->g->order(), m_solver->g->oldtype, m_solver->g->size());

	Solver solver;
	solver.setLimit(2);
	solver.loadEmpty(m_solver->g->rulset());

	if(createPartial(&solver)) {
		return true;
	}

// 	if(m_puzzle)
// 		return false;
// 	
// 	SKPuzzle* puzzle = new SKPuzzle(m_solver->g->order(), m_solver->g->oldtype, m_solver->g->size());
// 	
// 	if(!puzzle)
// 		return false;
// 	
// 	qDebug() << QString("Init a new game (difficulty %1, symmetry %2)").arg(difficulty).arg(symmetry);
// 
// 	Solver solver;
// 	solver.setLimit(1);
// 	solver.loadEmpty(m_solver->g->rulset());
// 	solver.solve();
// 	m_solver->problemToPuzzle(puzzle, &solver.results()[0]);
// 
// 	qDebug() << QString("Solved game (difficulty %1, symmetry %2)").arg(difficulty).arg(symmetry);
// 
// 	SKPuzzle* solution = 0;
// 	if(m_withSolution) {
// 		solution = new SKPuzzle(m_solver->g->order(), m_solver->g->oldtype);
// 		
// 		if(!solution) {
// 			delete puzzle;
// 			return false;
// 		}
// 		
// 		m_solver->copy(solution, puzzle);
// 	}
// 	
// 	m_solver->remove_numbers(puzzle, difficulty, symmetry, m_solver->g->oldtype); //why was it 1?
// 	m_difficulty = difficulty;
// 	m_symmetry   = symmetry  ;
// 	
// 	m_puzzle   = puzzle  ;
// 	m_solution = solution;

	return true;
}

int Puzzle::init(const QByteArray& values, int* forks) {
	if(m_puzzle)
		return -1;
	
	SKPuzzle* puzzle   = new SKPuzzle(m_solver->g->order(), m_solver->g->type());
	SKPuzzle* solution = new SKPuzzle(m_solver->g->order(), m_solver->g->type());
	
	if(!(puzzle && solution))
		return -1;
	
	for(int i = 0; i < m_solver->g->size(); ++i)
		puzzle->numbers[i] = values[i];
// 		puzzle->setValue(i, values[i]);
	
// 	m_solver->copy(solution, puzzle);

	Problem problem;
	m_solver->puzzleToProblem(&problem, puzzle);
	Solver solver;
	solver.setLimit(2);
	solver.loadProblem(&problem);
	solver.solve();
// 	int success = m_solver->solve(problem, 2, solution, forks);
	int success = solver.results().count();
	if(success == 0) {
		delete puzzle;
		delete solution;
		return 0;
	} else {
		m_solver->problemToPuzzle(solution, &solver.results()[0]);
	}

	m_puzzle = puzzle;
	if(m_withSolution)
		m_solution = solution;
	else
		delete solution;
	
	return success;
}

bool Puzzle::init(const QByteArray& values, const QByteArray& solutionValues) {
	if(m_puzzle)
		return false;
	
	
	SKPuzzle* puzzle = new SKPuzzle(m_solver->g->order(), m_solver->g->type());
	if(!puzzle)
		return false;
	
	for(int i = 0; i < m_solver->g->size(); ++i)
		puzzle->numbers[i] = values[i];
// 		puzzle->setValue(i, values[i]);
	
	if(solutionValues.count() != 0) {
		SKPuzzle* solution = new SKPuzzle(m_solver->g->order(), m_solver->g->type());
		if(!solution) {
			delete puzzle;
			return false;
		}
		
		for(int i = 0; i < m_solver->g->size(); ++i)
			solution->numbers[i] = solutionValues[i];
// 			solution->setValue(i, solutionValues[i]);
		
		m_solution = solution;
	}
	m_puzzle = puzzle;
	return true;
}


// QChar Puzzle::value2Char(uint value) const {
// 	// NOTE calls to this method are allowed before init
// 	return value + m_solver->g->zerochar;
// }
// 
// int Puzzle::char2Value(QChar c) const{
// 	// NOTE calls to this method are allowed before init
// 	int val = c.lower();
// 	if(val < m_solver->g->zerochar) return -1;
// 	val -= m_solver->g->zerochar;
// 	if((uint)val > (uint)m_solver->order) return -1;
// 	return val;
// }

}
