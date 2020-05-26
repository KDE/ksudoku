/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2015      Ian Wadham <iandw.au@gmail.com>                   *
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

#ifndef _KSUDOKUPUZZLE_H_
#define _KSUDOKUPUZZLE_H_

#include "skgraph.h"
#include "globals.h"

namespace ksudoku {
	
class Puzzle {
public:
	/**
	* @param[in] sovler       The solver used for this puzzle (the GameType) (not deleted when Puzzle is deleted)
	* @param[in] withSolution Whether a the solution for this puzzle should be stored
	*/
	explicit Puzzle(SKGraph* graph, bool withSolution = true);

public:
	/**
	* Creates a puzzle without any content
	*/
	bool init();
	/**
	* Creates a new puzzle based on the solver
	* @param[in] difficulty The difficulty of the new game.
	* @param[in] symmetry   The symmetry
	*/
	bool init(int difficulty, int symmetry);
	/**
	* Tries to create a game on existing values
	* @param[in]  values The values used for the new puzzle
	* @param[out] forks  The count of forks did solving the puzzle
	* @return <0 on error, 0 for no solutions, 1 for exactly one solution and
	* 2 for more than 1 solutions. For return value <= 0 the init failed.
	*/
	int init(const BoardContents& values);
	
	/**
	* Return game type
	*/
	GameType gameType() const {
		return m_graph->type();
	}

	int value(int index) const;
	int solution(int index) const;

	/**
	 * Get the index of the n'th move in the solution, for use as a hint.
	 * @param moveNum   The number of the move for which a hint is required.
	 * @return          The position of the move, or -1 if past end of list.
	 */
	int hintIndex(int moveNum) const;

	inline bool hasSolution() const { return (m_solution.size() > 0); }

	///@return order of game
	int order() const { return m_graph->order(); }
	
	///@return total elements in puzzle
	int size() const { 
		return m_graph->sizeX() * m_graph->sizeY() * m_graph->sizeZ(); 
		}

	///create new Puzzle with same solver and set withSolution to true
	Puzzle* dubPuzzle() { return new Puzzle(m_graph, true) ; }

public:
	inline SKGraph *graph() const { return m_graph; }

private:
	bool m_withSolution;
	SKGraph *m_graph;

	BoardContents m_puzzle;
	BoardContents m_solution;
	QList<int> m_hintList;

	bool m_initialized;
};

}

#endif
