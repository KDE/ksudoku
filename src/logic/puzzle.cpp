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
/*
#include <choiceitem.h>
*/

#include "sudokuboard.h"
#include "plainsudokuboard.h"
#include "samuraiboard.h"
#include "roxdokuboard.h"

namespace ksudoku {

Puzzle::Puzzle(SKGraph *graph, bool withSolution)
	: m_withSolution(withSolution)
	, m_graph(graph)
	/* , m_alternateSolver(false) */
	, m_difficulty(0)
	, m_symmetry(0)
	, m_initialized(false)
{ }

int Puzzle::value(int index) const {
/*	if (m_alternateSolver) { */
	return ((index < m_puzzle.size()) ? m_puzzle.at(index) : 0);
/*
	}
	Item *item = m_graph->board()->itemAt(m_graph->cellPosX(index), m_graph->cellPosY(index), m_graph->cellPosZ(index));
	if(item && m_puzzle2.ruleset())
		return static_cast<ChoiceItem*>(item)->value(&m_puzzle2);
	return 0;
*/
}

int Puzzle::solution(int index) const {
/*	if (m_alternateSolver) { */
	return ((index < m_solution.size()) ? m_solution.at(index) : 0);
/*
	}
	Item *item = m_graph->board()->itemAt(m_graph->cellPosX(index), m_graph->cellPosY(index), m_graph->cellPosZ(index));
	if(item && m_solution2.ruleset())
		return static_cast<ChoiceItem*>(item)->value(&m_solution2);
	return 0;
*/
}

bool Puzzle::init() {
	if(m_initialized) return false;
	
	if(m_withSolution)
		return false;
/*
	m_alternateSolver = true;

	m_alternateSolver = false;
	m_puzzle2 = Problem(m_graph->rulset());
*/

	return true;
}

/*
bool Puzzle::createPartial(Solver *solver) {
	// TODO after finding a solution try to find simpler ones
	const ItemBoard *board = m_graph->board();
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
				m_solution2 = solver->results()[0];
			solver->pop();
			
			if(count == 1) {
				m_puzzle2 = solver->state();
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
*/

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
/*
	Solver solver;
	solver.setLimit(2);
	solver.loadEmpty(m_graph->rulset());

	if(createPartial(&solver)) {

		QByteArray bytes;
		for (int n = 0; n < size(); n++) {
		    qDebug() << "Index" << n << "x =" << m_graph->cellPosX(n) <<
						"y =" << m_graph->cellPosY(n) <<
						"z =" << m_graph->cellPosZ(n) <<
						"value" << (char) (value(n) + '0');
		    bytes.append (value(n) + '0');
		}

		qDebug() << "ALL VALUES" << bytes;
	
		return true;
	}

	return false;
*/
}

int Puzzle::init(const QByteArray& values) {
    QByteArray out = values; // IDW test.
    for (int i = 0; i < out.size(); i++) out[i] = out.at(i) + '0'; // IDW test.
    qDebug() << "Values" << out; // IDW test.
	if(m_initialized) return -1;
/*
	m_alternateSolver = false;
	setValues(values);
	
	Solver solver;
	solver.setLimit(2);
	solver.loadProblem(&m_puzzle2);
	solver.solve();

	int success = solver.results().count();
	if(success == 1 && m_withSolution) {
		m_solution2 = solver.results()[0];
	}
	
*/
	int success = 1; // TODO - IDW - Get SudokuBoard to check the solution.
	return success;
}

/*
void Puzzle::setValues(const QByteArray& values) {
	qDebug() << "Puzzle::init, values size:" << values.size() << "XYZ size"
	         << m_graph->sizeX() << m_graph->sizeY() << m_graph->sizeZ();
	m_puzzle2 = Problem(m_graph->rulset());
	for(int x = 0; x < m_graph->sizeX(); ++x) {
		for(int y = 0; y < m_graph->sizeY(); ++y) {
			for(int z = 0; z < m_graph->sizeZ(); ++z) {
				int value = values[m_graph->cellIndex(x, y, z)];
				Item *item = m_graph->board()->itemAt(x, y, z);
				if(item && value) {
					static_cast<ChoiceItem*>
					    (item)->setValue(&m_puzzle2, value);
				}
			}
		}
	}
}
*/

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
