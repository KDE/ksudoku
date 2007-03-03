//
// C++ Interface: PuzzleFactory
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
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
