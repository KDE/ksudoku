/***************************************************************************
 *   Copyright 2007      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#ifndef _KSUDOKUSOLVER_H_
#define _KSUDOKUSOLVER_H_

#include "skgraph.h"
#include <QVector>
#include <QBitArray> // TODO Remove (only needed by SolverState)

#include "solverstate.h"
		
class SKSolver;

namespace ksudoku {

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
	
	int solve(const QVector<int>& puzzle, int maxSolutions = 1);
// 	bool removeNumbers(const QValueVector<uint>& puzzle);
	
	QVector<int> result() const { return m_result; }
	
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
	QVector<int> m_result;
};

}

#endif
