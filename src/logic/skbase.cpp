/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
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
	base = (int) sqrt((double)order);
	
	if(sized!=-1) size=sized;
	else size = (type==1) ?  base*base*base :  (order*order) ;
	if(order>9) zerochar = 'a'-1; else zerochar = '0';
}


