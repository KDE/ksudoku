//
// C++ Interface: pagesize
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

	///@return a QRect describing the heigth and width of @c name
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
