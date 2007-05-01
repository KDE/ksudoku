/***************************************************************************
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

#include "drawfactory.h"

#include "ksudoku_types.h"
#include "puzzle.h"
#include "sudokudraw.h"
#include "roxdokudraw.h"

namespace ksudoku {

DrawFactory::DrawFactory()
{
}

DrawFactory::~DrawFactory()
{
}

DrawBase* DrawFactory::create_instance(Puzzle const& puzzle, Symbols const& symbols) const
{
	switch(puzzle.gameType()){
		case sudoku:
			return new SudokuDraw(puzzle, symbols);
			break;
		case roxdoku:
			///@TODO enable roxdoku prints or give warning
			return 0;//new RoxdokuDraw(puzzle, symbols);
			break;
		default:
			return 0;
			break;
	}
}


}
