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

Puzzle::Puzzle(SKGraph *graph, bool withSolution)
	: m_withSolution(withSolution)
	, m_graph(graph)
	, m_difficulty(0)
	, m_symmetry(0)
	, m_initialized(false)
{ }

int Puzzle::value(int index) const {
	Item *item = m_graph->board()->itemAt(m_graph->cellPosX(index), m_graph->cellPosY(index), m_graph->cellPosZ(index));
	if(item && m_puzzle2.ruleset())
		return static_cast<ChoiceItem*>(item)->value(&m_puzzle2);
	return 0;
}

int Puzzle::solution(int index) const {
	Item *item = m_graph->board()->itemAt(m_graph->cellPosX(index), m_graph->cellPosY(index), m_graph->cellPosZ(index));
	if(item && m_solution2.ruleset())
		return static_cast<ChoiceItem*>(item)->value(&m_solution2);
	return 0;
}

bool Puzzle::init() {
	if(m_initialized) return false;
	
	if(m_withSolution)
		return false;

	m_puzzle2 = Problem(m_graph->rulset());

	return true;
}

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

bool Puzzle::init(int difficulty, int symmetry) {
	if(m_initialized) return false;
	Solver solver;
	solver.setLimit(2);
	solver.loadEmpty(m_graph->rulset());

	if(createPartial(&solver)) {
		return true;
	}

	return false;
}

int Puzzle::init(const QByteArray& values) {
	if(m_initialized) return -1;
	
	m_puzzle2 = Problem(m_graph->rulset());
	for(int x = 0; x < m_graph->sizeX(); ++x) {
		for(int y = 0; y < m_graph->sizeY(); ++y) {
			for(int z = 0; z < m_graph->sizeZ(); ++z) {
				int value = values[m_graph->cellIndex(x, y, z)];
				Item *item = m_graph->board()->itemAt(x, y, z);
				if(item && value)
					static_cast<ChoiceItem*>(item)->setValue(&m_puzzle2, value);
			}
		}
	}
	
	Solver solver;
	solver.setLimit(2);
	solver.loadProblem(&m_puzzle2);
	solver.solve();

	int success = solver.results().count();
	if(success == 1 && m_withSolution) {
		m_solution2 = solver.results()[0];
	}
	
	return success;
}

}
