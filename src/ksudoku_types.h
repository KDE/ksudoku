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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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

