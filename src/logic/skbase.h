//
// C++ Interface: skbase
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SKBASE_H
#define SKBASE_H

#define ITERATE(i,s) for(int ((i))=0; ((i))<((s)); ++((i)))

/**
	@author 
*/

class SKBase
{
public:
	int base;
	int order;
	int size;
	char zerochar;
	int type;
	void setorder(int k, int type = 0, int sized=-1);
	SKBase(int i=9, int typef = 0,int sized=-1)
		{setorder(i, typef,sized); };
};

#endif
