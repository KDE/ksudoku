//
// C++ Interface: ksudoku_types
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _KSUDOKU_TYPES_H_
#define _KSUDOKU_TYPES_H_


namespace ksudoku {

//UserEvent is defined as User in qevent.h since it is very unlikely
//to change and for faster compilation (...) it is defined here as USER_EVENT
#define USER_EVENT 1000
#define GENERATE_EVENT USER_EVENT+1


enum ButtonState {
	GivenValue     = 0,
	CorrectValue   = 1,
	WrongValue     = 2,
	ObviouslyWrong = 3,
	Marker         = 4
};

enum GameType {
	sudoku,
	roxdoku,
	custom
};

}

#endif

