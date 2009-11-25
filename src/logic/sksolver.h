/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
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

#ifndef SKSolver_H
#define SKSolver_H

#include "skpuzzle.h"
#include "skgraph.h"

#include "ksudoku_types.h"

#define RANDOM(x) (int) (x*((float)rand())/(RAND_MAX+1.0f))

#define LEVINC         0
#define SIMMETRY_RANDOM 0
#define SIMMETRY_NONE 1
#define SIMMETRY_DIAGONAL 2
#define SIMMETRY_CENTRAL 3
#define SIMMETRY_FOURWAY 4

#include <QVector>

#include "solver.h"


using namespace ksudoku;

typedef unsigned int uint;

class Problem;

#include <math.h>
// SUDOKU PUZZLE SOLVER BASIC ALGORITHM - BY FRANCESCO ROSSI 2005 redsh@email.it

/**
@author Francesco Rossi
*/

class SKSolver {
public:
	~SKSolver();
	explicit SKSolver(SKGraph* gr);
public:
	SKGraph* g;
public:
	int remove_numbers (SKPuzzle* p, int difficulty, int simmetry, int type);
	int solve (const Problem &problem,  int max_solutions = 1, SKPuzzle* out_solutions = 0, int* forks=0);

	void copy(SKPuzzle* dest, SKPuzzle* src);

	bool puzzleToProblem(Problem *dest, SKPuzzle *source) const;
	bool problemToPuzzle(SKPuzzle *dest, Problem *source) const;
	
private:
	int get_simmetric(int order, int size, int type, int idx, int which, int out[4]);
};

#endif
