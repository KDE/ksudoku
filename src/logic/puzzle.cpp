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

#include "sudokuboard.h"

namespace ksudoku {

Puzzle::Puzzle(SKGraph *graph, bool withSolution)
	: m_withSolution(withSolution)
	, m_graph(graph)
	, m_difficulty(0)
	, m_symmetry(0)
	, m_initialized(false)
{ }

int Puzzle::value(int index) const {
	return ((index < m_puzzle.size()) ? m_puzzle.at(index) : 0);
}

int Puzzle::solution(int index) const {
	return ((index < m_solution.size()) ? m_solution.at(index) : 0);
}

int Puzzle::hintIndex(int moveNum) const {
	return ((moveNum >= m_hintList.count()) ? -1 : m_hintList.at(moveNum));
}

bool Puzzle::init() {
	if(m_initialized) return false;
	
	if(m_withSolution)
		return false;

	return true;
}

bool Puzzle::init(int difficulty, int symmetry) {
	if(m_initialized) return false;

	SudokuBoard * board = new SudokuBoard (m_graph);

	// Generate a puzzle and its solution.
	qDebug() << "Calling board->generatePuzzle()"; // IDW test.
	board->generatePuzzle (m_puzzle, m_solution,
			      (Difficulty) difficulty, (Symmetry) symmetry);
	board->getMoveList (m_hintList);
	delete board;
	return true;
}

int Puzzle::init(const BoardContents & values) {
	if(m_initialized) return -1;

	SudokuBoard * board = new SudokuBoard (m_graph);

	// Save the puzzle values and SudokuBoard's solution (if any).
	m_puzzle = values;
	m_solution = board->solveBoard (m_puzzle);

	// Get SudokuBoard to check the solution.
	int result = board->checkPuzzle (m_puzzle);
	if (result >= 0) {
	    result = 1;		// There is one solution.
	}
	else if (result == -1) {
	    result = 0;		// There is no solution.
	}
	else {
	    result = 2;		// There is more than one solution.
	}
	m_hintList.clear();
	if (result != 0) {
	    board->getMoveList (m_hintList);
	}
	delete board;
	return result;
}

}
