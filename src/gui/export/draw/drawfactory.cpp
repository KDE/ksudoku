//
// C++ Implementation: drawfactory
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
