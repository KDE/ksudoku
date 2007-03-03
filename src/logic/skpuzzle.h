//
// C++ Interface: skpuzzle
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

	SKPuzzle(int oi=9, int typef = 0, int sized=-1) : SKBase(oi,typef,sized )
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
