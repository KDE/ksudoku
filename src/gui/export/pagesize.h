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

#ifndef KSUDOKUPAGESIZE_H
#define KSUDOKUPAGESIZE_H

#include <qstringlist.h>
#include <qsize.h>
#include <qmap.h>


namespace ksudoku {


/**
 * Convert default page size (A4,B5 etc.) to QRect and vice versa.
 * Also provides a list containing the available defaults (incl custom).
*/
class PageSize{
	typedef QMap<QString, QSize> SizeMap;

public:
	PageSize();
	~PageSize();

	QStringList const& pageSizeNames() const { return m_nameList; }

	///returns index to location of @a name in @c m_nameList
	///@returns the index of the first occurrence of the value x. Returns -1 if no item matched.
	int index(QString const& name) const { return m_nameList.indexOf(name); }

	///@return a QRect describing the height and width of @c name
	QSize size(QString const& name) const;
	///over loaded from function above
	///@arg index to a pageSize in m_nameList
	QSize size(int index) const { return size(m_nameList[index]); }

	///Compare internal size of @a type with @a size (size must be given in mm)
	///@returns true if equal, false otherwise
	bool equal(QString const& type, QSize size) { return size == PageSize::size(type); }

private:
	///fill m_indexNames with name index pairs
	void init();
	void add(QString const name, QSize const& value);

	///holds name/index pairs
	SizeMap m_sizeList;

	///holds paper size names stored in m_indexNames, but now in predefinde order
	QStringList m_nameList;
};

}

#endif
