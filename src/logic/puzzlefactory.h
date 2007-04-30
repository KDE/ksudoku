/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef KSUDOKUPUZZLEFACTORY_H
#define KSUDOKUPUZZLEFACTORY_H

#include "puzzle.h"
//#include "ksudokugame.h"
#include "ksudoku_types.h"


namespace ksudoku {


/**
	Generate Puzzles
	Caller will own (and delete) generated puzzle
*/
class PuzzleFactory{
public:
	PuzzleFactory();
	~PuzzleFactory();

	///create a puzzle
	///@warning caller is responsable for deletion
	Puzzle* create_instance(GameType type, int order, int difficulty, int symmetry, bool dub = false, SKSolver* s = NULL);
};

}

#endif
