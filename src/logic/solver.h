/* This file is part of the ksudoku project
   Copyright (C) 2007 Johannes Bergmeier <Johannes.Bergmeier@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KSUDOKUSOLVER_H_
#define _KSUDOKUSOLVER_H_

#include "skgraph.h"
#include <q3valuevector.h>
#include <qbitarray.h> // TODO Remove (only needed by SolverState)
#include "grouplookup.h" // TODO Remove (only needed by SolverState)
		
class SKSolver;

namespace ksudoku {

class SolverState;

enum ProcessState {
	/// An operation finished successfuly
	KSS_SUCCESS          = 0,
	/// An operation failed, this may (dependant on op) mean that there is no solution
	KSS_FAILURE          = 1,
	/// An operation stopped, as no more solutions are required
	KSS_ENOUGH_SOLUTIONS = 2,
	/// To count of maximum forks was exceeded
	KSS_ENOUGH_FORKS     = 3,
	/// There is an internal failure
	KSS_CRITICAL         = 4
};

enum DifficultyFlags {
	KSS_SYM_NONE     = 0,
	KSS_SYM_DIAGONAL = 1,
	KSS_SYM_CENTRAL  = 2,
	KSS_SYM_FOURWAY  = KSS_SYM_DIAGONAL | KSS_SYM_CENTRAL,
	KSS_SYM_MASK     = KSS_SYM_FOURWAY,
	KSS_REM_1VALUE   = 4
};

typedef SKGraph Graph;

class Solver {
public:
	explicit Solver(Graph* graph, uint flags = 0);
	~Solver();
	
// 	bool createNewPuzzle();
	
	int solve(const Q3ValueVector<int>& puzzle, int maxSolutions = 1);
// 	bool removeNumbers(const QValueVector<uint>& puzzle);
	
	Q3ValueVector<int> result() const { return m_result; }
	
private:
	friend class ::SKSolver;
	// TODO add this to the new graph class
	int getSymmetricIndices(int index, int out[4]);
	
	// Solve Methods
	ProcessState solveByLastFlag(SolverState& state);
	ProcessState solveByForks(SolverState& state);
	
	
private:
	// The use of members in SKSolver is only temporal, there is no need for a accessors
	int m_solutionsLeft;
	int m_forksLeft;
	Graph* m_graph;
	uint m_flags;
	Q3ValueVector<int> m_result;
};

}

#endif
