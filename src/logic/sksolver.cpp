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

#include "sksolver.h"

#include <stdio.h>
// #include <cstdlib>

#include <time.h>

// SUDOKU PUZZLE SOLVER BASIC ALGORITHM - BY FRANCESCO ROSSI 2005 redsh@email.it

// #include <qtextstream.h>
#include <qbitarray.h>

#include <iostream>

#include "solver.h"

using namespace ksudoku;
using namespace std;

//
// class SKSolver
//

SKSolver::SKSolver(SKGraph* gr)
{
	g = gr;
}

SKSolver::~SKSolver()
{
	delete g;
}
