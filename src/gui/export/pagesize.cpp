/***************************************************************************
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "pagesize.h"


#include <klocale.h>


namespace ksudoku {


void PageSize::init()
{
	//names and values copied from QPrinter documentation
	//unit is mm
	add(i18n("A0"), QSize(841, 1189)); // This value is not supported on windows. 
	add(i18n("A1"), QSize(594, 841));  // This value is not supported on windows. 
	add(i18n("A2"), QSize(420, 594));
	add(i18n("A3"), QSize(297, 420));
	add(i18n("A4"), QSize(210, 297));  //, 8.26, 11.7 inches 
	add(i18n("A5"), QSize(148, 210));
	add(i18n("A6"), QSize(105, 148));
	add(i18n("A7"), QSize(74, 105));
	add(i18n("A8"), QSize(52, 74));
	add(i18n("A9"), QSize(37, 52));
	add(i18n("B0"), QSize(1030, 1456));
	add(i18n("B1"), QSize(728, 1030));
	add(i18n("B10"), QSize(32, 45));
	add(i18n("B2"), QSize(515, 728));
	add(i18n("B3"), QSize(364, 515));
	add(i18n("B4"), QSize(257, 364));
	add(i18n("B5"), QSize(182, 257)); //, 7.17, 10.13 inches 
	add(i18n("B6"), QSize(128, 182));
	add(i18n("B7"), QSize(91, 128));
	add(i18n("B8"), QSize(64, 91));
	add(i18n("B9"), QSize(45, 64));
	add(i18n("C5E"), QSize(163, 229));
	add(i18n("Comm10E"), QSize(105, 241)); //, US Common #10 Envelope 
	add(i18n("DLE"), QSize(110, 220));
//can't use doulbe   add(i18n("Executive"), QSize(7.5, 10)); // inches, 191, 254)
	add(i18n("Folio"), QSize(210, 330));
	add(i18n("Ledger"), QSize(432, 279));
	add(i18n("Legal"), QSize(216, 356)); // 8.5, 14 inches,
	add(i18n("Letter"), QSize(216, 279)); // 8.5, 11 inches
	add(i18n("Tabloid"), QSize(279, 432));
	//added for convenience (schould be hidden becouse it does not 
	//provide a real size
	add(i18n("Custom"), QSize(-1, -1));
};


PageSize::PageSize()
{
	init();
}

void PageSize::add(QString const name, QSize const& value)
{
	m_sizeList[name] = value;
	m_nameList.push_back(name);
}


PageSize::~PageSize()
{
}

QSize PageSize::size(QString const& name) const
{ 
	SizeMap::const_iterator iter = m_sizeList.find(name);
	if(iter != m_sizeList.end())
		return *iter;
	else
		return QSize(-1,-1); //invalid qsize. Better would be to throw an error or 
		                     //give a warning. Actually, ending up here is a bug
}


}
