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

#include "sudoku_solver.h"
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
	
	/**
	* @todo Delete puzzle and solver when this class is not only used for puzzle creation
	*/
	~Puzzle();
	
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
	int init(const QByteArray& values, int* forks = 0);
	/**
	* Create a puzzle based on complete data
	* @param[in] values   The values used for the new puzzle
	* @param[in] solution The values used for the solution of the new puzzle
	* @note If you don't need a solution simply pass QByteArray() as solution.
	*/
	bool init(const QByteArray& values, const QByteArray& solutionValues);
	
//		/**
// 		 * Gets the corresponding character for a @p value
// 		 * @note This method can be called before @c init was called
// 		 */
// 		QChar value2Char(uint value) const;

// 		/**
// 		 * Gets the value for a character @p c.
// 		 * If @p c has no corresponding value a value < 0 will be returned.
// 		 * @note This method can be called before @c init was called
// 		 */
// 		int char2Value(QChar c) const;

	/**
	* Return game type
	*/
	GameType gameType() const {
		return (m_solver->g->type==0) ? sudoku : (m_solver->g->type==1 ? roxdoku : custom); 
	}

// 		inline uint value(uint index) const { return m_puzzle ? m_puzzle->value(index) : 0; }
// 		inline uint value(uint x, uint y, uint z = 0) const { return value(index(x,y,z)); }
// 		inline uint solution(uint index) const { return m_solution ? m_solution->value(index) : 0; }
	inline int value(int index) const { return m_puzzle ? m_puzzle->numbers[index] : 0; }
	inline int value(int x, int y, int z = 0) const { return value(index(x,y,z)); }
	inline int solution(int index) const { return m_solution ? m_solution->numbers[index] : 0; }
	
	inline bool hasSolution() const { return m_withSolution && m_solution; }

	///convert coordinates in a puzzle to one index value
	int index(int x, int y, int z = 0) const {
		if(!m_solver) return 0;
		return (x*m_solver->g->sizeY() + y)*m_solver->g->sizeZ() + z;
	}

	///@return order of game
	int order() const { return m_solver->g->order; }
	
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
	inline SKPuzzle* puzzle()   const { return m_puzzle  ; }
	inline SKPuzzle* solution() const { return m_solution; }
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
	bool m_withSolution;
	SKPuzzle* m_puzzle;
	SKPuzzle* m_solution;
	SKSolver* m_solver;

	int m_difficulty;
	int m_symmetry  ;
};

}

#endif
