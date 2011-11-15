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
#include "plainsudokuboard.h"
#include "samuraiboard.h"
#include "roxdokuboard.h"

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

bool Puzzle::init() {
	if(m_initialized) return false;
	
	if(m_withSolution)
		return false;

	return true;
}

bool Puzzle::init(int difficulty, int symmetry) {
	if(m_initialized) return false;

	QObject * owner = new QObject();	// Stands in for "this".
	SudokuBoard * board = getBoard (owner);
	if (board == 0) {
	    return false;
	}

	// Generate a puzzle and its solution.
	BoardContents puzzle;
	BoardContents solution;
	board->generatePuzzle (puzzle, solution,
			      (Difficulty) difficulty, (Symmetry) symmetry);
	int boardSize = board->boardSize();
	delete owner;			// Also deletes the SudokuBoard object.

	// Convert the puzzle and solution to KSudoku format.
	m_puzzle   = convertBoardContents(puzzle, boardSize);
	m_solution = convertBoardContents(solution, boardSize);

	return true;
}

int Puzzle::init(const QByteArray& values) {
	if(m_initialized) return -1;

	QObject * owner = new QObject();	// Stands in for "this".
	SudokuBoard * board = getBoard (owner);
	if (board == 0) {
	    return 0;
	}

	// The new solver stores by column within row.
	// KSudoku stores values by row within column.
	BoardContents puzzleValues;
	int boardSize = board->boardSize();
	for (int j = 0; j < boardSize; j++) {
	    for (int i = 0; i < boardSize; i++) {
		puzzleValues.append((int) values.at(i * boardSize + j));
	    }
	}
	qDebug() << puzzleValues.size() << "puzzleValues" << puzzleValues;
	board->print(puzzleValues);

	// Save the puzzle values and SudokuBoard's solution (if any).
	m_puzzle = values;
	m_solution = convertBoardContents
			(board->solveBoard(puzzleValues), boardSize);

	// Get SudokuBoard to check the solution.
	int result = board->checkPuzzle (puzzleValues);
	if (result >= 0) {
	    result = 1;		// There is one solution.
	}
	else if (result == -1) {
	    result = 0;		// There is no solution.
	}
	else {
	    result = 2;		// There is more than one solution.
	}
	delete owner;		// Also deletes the SudokuBoard object.
	return result;
}

const QByteArray Puzzle::convertBoardContents(const BoardContents & values,
					      int boardSize) {
	// New solver stores by column within row and sets unused cells = -1.
	// KSudoku stores values by row within column and sets unused cells = 0,
	// e.g. the empty areas in a Samurai or Tiny Samurai puzzle layout.
	QByteArray newValues;
	char value = 0;
	int index = 0;
	for (int j = 0; j < boardSize; j++) {
	    for (int i = 0; i < boardSize; i++) {
		index = i * boardSize + j;
		value = values.at(index) < 0 ? 0 : values.at(index);
		newValues.append(value);
	    }
	}
	return newValues;
}

SudokuBoard * Puzzle::getBoard(QObject * owner) {
	int blockSize = 3;
	for (int n = 2; n <= 5; n++) {
	    if (m_graph->order() == n * n) {
		blockSize = n;
	    }
	}
	SudokuType type = m_graph->specificType();
	qDebug() << "Type" << type; // IDW test.
	SudokuBoard * board = 0;
	// Generate a puzzle and solution of the required type.
	switch (type) {
	case Plain:
	    board = new PlainSudokuBoard (owner, type, blockSize);
	    break;
	case XSudoku:
	    board = new XSudokuBoard (owner, type, blockSize);
	    break;
	case Jigsaw:
	    board = new JigsawBoard (owner, type, blockSize);
	    break;
	case Samurai:
	    board = new SamuraiBoard (owner, type, blockSize);
	    break;
	case TinySamurai:
	    board = new TinySamuraiBoard (owner, type, blockSize);
	    break;
	case Roxdoku:
	    board = new RoxdokuBoard (owner, type, blockSize);
	    break;
	case EndSudokuTypes:
	    return 0;
	    break;
	}
	board->setUpBoard();
	return board;
}

}
