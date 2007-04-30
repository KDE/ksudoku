/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SKPUZZLE_H
#define SKPUZZLE_H

#include "skbase.h"

/**
	@author 
*/

class SKPuzzle : public SKBase
{
public:
	unsigned char numbers[625];
	unsigned char flags[625][26];

	explicit SKPuzzle(int oi=9, int typef = 0, int sized=-1) : SKBase(oi,typef,sized )
	{
		/*numbers = new (unsigned char ) [size+1];
		flags   = new (unsigned char*) [size];
		ITERATE(j,size) flags[j] = new unsigned char[order+1];*/

		ITERATE(i,size)
		{
			numbers[i]=0; 
			ITERATE(j,order+1) flags[i][j]=1;
		}
		
	}
	~SKPuzzle()
	{
		/*delete numbers;
		delete []  flags;*/
		
	}
};

#endif
