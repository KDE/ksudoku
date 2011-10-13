/****************************************************************************
 *    Copyright 2009  Ian Wadham <iandw.au@gmail.com>                       *
 *                                                                          *
 *    This program is free software; you can redistribute it and/or         *
 *    modify it under the terms of the GNU General Public License as        *
 *    published by the Free Software Foundation; either version 2 of        *
 *    the License, or (at your option) any later version.                   *
 *                                                                          *
 *    This program is distributed in the hope that it will be useful,       *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *    GNU General Public License for more details.                          *
 *                                                                          *
 *    You should have received a copy of the GNU General Public License     *
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ****************************************************************************/

#ifndef KGRDEBUG_H
#define KGRDEBUG_H

static int dbgLevel = 0;	// Local to file where kgrdebug.h is included.

#define dbk  kDebug()
#define dbk1 if(dbgLevel>=1)kDebug()
#define dbk2 if(dbgLevel>=2)kDebug()
#define dbk3 if(dbgLevel>=3)kDebug()

#define dbo  printf(
#define dbo1 if(dbgLevel>=1)printf(
#define dbo2 if(dbgLevel>=2)printf(
#define dbo3 if(dbgLevel>=3)printf(

#define dbe  fprintf(stderr,
#define dbe1 if(dbgLevel>=1)fprintf(stderr,
#define dbe2 if(dbgLevel>=2)fprintf(stderr,
#define dbe3 if(dbgLevel>=3)fprintf(stderr,

#endif
