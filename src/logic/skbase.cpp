//
// C++ Implementation: skbase
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "skbase.h"

#include <math.h>

//SKBase::SKBase()
//{
//}


//SKBase::~SKBase()
//{
//}


void SKBase::setorder(int k, int threedimensionalf, int sized)
{
	type = threedimensionalf;
	order = k; 
	base = (int) sqrt(order);
	
	if(sized!=-1) size=sized;
	else size = (type==1) ?  base*base*base :  (order*order) ;
	if(order>9) zerochar = 'a'-1; else zerochar = '0';
}


