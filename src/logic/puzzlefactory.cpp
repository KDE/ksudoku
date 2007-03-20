//
// C++ Implementation: PuzzleFactory
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
