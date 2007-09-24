/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
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

#include "puzzlefactory.h"

#include "puzzle.h"


namespace ksudoku {


PuzzleFactory::PuzzleFactory()
{
}


PuzzleFactory::~PuzzleFactory()
{
}

Puzzle* PuzzleFactory::create_instance(GameType type, int order, int difficulty, int symmetry, bool dub, SKSolver* customsolver){
	SKSolver* solver = 0;

	bool d3 = (type == roxdoku) ? 1 : 0; //3d or not 3d

	switch(order){
		case  0:
		case  9:
		case 16:
		case 25:
			if(type==ksudoku::custom)
			{
				order = customsolver->order;
				solver = customsolver;
			}
			else
			{
				solver = new SKSolver(order, d3);
				solver->init();
			}
			break;
		default:
			///@todo decide what to do here
			return 0;
			break;
	}
	
	Puzzle* puzzle = new Puzzle(solver, !dub);
	
	if(!dub) {
		if(!puzzle->init(difficulty, symmetry)) {
			delete puzzle;
			return 0;
		}
	}else {
		if(!puzzle->init()) {
			delete puzzle;
			return 0;
		}
	}
	return puzzle;
}


}
