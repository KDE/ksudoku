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

#ifndef _KSUDOKUPUZZLE_H_
#define _KSUDOKUPUZZLE_H_

#include "sksolver.h"
#include "ksudoku_types.h"

class QChar;

namespace ksudoku {
	
class Puzzle {
public:
	/**
	* @param[in] sovler       The solver used for this puzzle (the GameType) (not deleted when Puzzle is deleted)
	* @param[in] withSolution Whether a the solution for this puzzle should be stored
	*/
	explicit Puzzle(SKSolver* solver, bool withSolution = true);
	
public:
	/**
	* Creates a puzzle without any content
	*/
	bool init();
	/**
	* Creates a new puzzle based on the solver
	* @param[in] difficulty The difficulty of the new game (for valid values see
	*                       code of @c SKSolver)
	* @param[in] symmetry   The symmetry (for valid values see code of @c SKSolver)
	*/
	bool init(int difficulty, int symmetry);
	/**
	* Tries to create a game on existing values
	* @param[in]  values The values used for the new puzzle
	* @param[out] forks  The count of forks did solving the puzzle
	* @return <0 on error, 0 for no solutions, 1 for exactly one solution and
	* 2 for more than 1 solutions. For return value <= 0 the init failed.
	*/
	int init(const QByteArray& values);
	
	/**
	* Return game type
	*/
	GameType gameType() const {
// 		return (m_solver->g->oldtype==0) ? TypeSudoku : (m_solver->g->oldtype==1 ? TypeRoxdoku : TypeCustom);
		return m_solver->g->type();
	}

	int value(int x, int y, int z = 0) const;
	inline int value(int index) const { return value(m_solver->g->cellPosX(index), m_solver->g->cellPosY(index), m_solver->g->cellPosZ(index)); }
	int solution(int x, int y, int z = 0) const;
	inline int solution(int index) const { return solution(m_solver->g->cellPosX(index), m_solver->g->cellPosY(index), m_solver->g->cellPosZ(index)); }

	inline bool hasSolution() const { return m_withSolution && m_solution2.ruleset(); }

	///convert coordinates in a puzzle to one index value
	int index(int x, int y, int z = 0) const {
		if(!m_solver) return 0;
		return m_solver->g->cellIndex(x, y, z);
	}

	///@return order of game
	int order() const { return m_solver->g->order(); }
	
	///@return total elements in puzzle
	int size() const { 
		return m_solver->g->sizeX() * m_solver->g->sizeY() * m_solver->g->sizeZ(); 
		}

	int optimized_d(int index) const { return m_solver->g->optimized_d[index]; }
	int optimized(int indX, int indY) const { return m_solver->g->optimized[indX][indY]; }
	bool hasConnection(int i, int j) const { return m_solver->g->hasConnection(i,j); }

	///@return if Puzzle has a solver or not
	bool hasSolver() { return (m_solver == 0) ? false : true; }

	///create new Puzzle with same solver and set withSolution to true
	Puzzle* dubPuzzle() { return new Puzzle(m_solver,true) ; }

public:
	inline SKSolver* solver  () const { return m_solver  ; }

	///return value used for difficulty setting.
	///@WARNING only valid after init(int difficulty, int symmetry)
	///         has been called
	inline int difficulty() const { return m_difficulty; }
	///return value used for symmetry setting.
	///@WARNING only valid after init(int difficulty, int symmetry)
	///         has been called
	inline int symmetry() const { return m_symmetry; }

private:
	bool createPartial(Solver* graph);

private:
	bool m_withSolution;
	SKSolver* m_solver;
	
	Problem m_puzzle2;
	Problem m_solution2;

	int m_difficulty;
	int m_symmetry;

	bool m_initialized;
};

}

#endif
