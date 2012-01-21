/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2012      Ian Wadham <iandw.au@gmail.com>                   *
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

#ifndef SKGRAPH_H
#define SKGRAPH_H

#include <QString>
#include <QVector>

#include "ksudoku_types.h"
#include "globals.h"

class SKGraph
{
public:
	explicit SKGraph(int order = 9, ksudoku::GameType type = ksudoku::TypeSudoku);
	virtual ~SKGraph();

	inline int sizeX() const { return m_sizeX; }
	inline int sizeY() const { return m_sizeY; }
	inline int sizeZ() const { return m_sizeZ; }

	inline int size() const { return m_sizeX * m_sizeY * m_sizeZ; }

	inline int cellIndex(uint x, uint y, uint z = 0) const
	{
		return (x*sizeY() + y)*sizeZ() + z;
	}
	inline int cellPosX(int i) const {
		if(!(sizeX() && sizeY() && sizeZ())) return 0;
		return i/sizeZ()/sizeY();
	}
	inline int cellPosY(int i) const {
		if(!(sizeX() && sizeY() && sizeZ())) return 0;
		return i/sizeZ()%sizeY();
	}
	inline int cellPosZ(int i) const {
		if(!(sizeX() && sizeY() && sizeZ())) return 0;
		return i%sizeZ();
	}

	inline int cliqueCount() const { return m_cliques.count(); }

	inline const QString & name() const { return m_name; }
	inline int order() const { return m_order; }
	inline ksudoku::GameType type() const { return m_type; }
	inline int base() const { return m_base; }

	virtual SudokuType specificType() const { return m_specificType; }

	QVector<int> clique(int i) const { return m_cliques[i]; }

	void initSudoku();

	void initRoxdoku();

	void initCustom(const QString & name, SudokuType specificType,
		  int order, int sizeX, int sizeY, int sizeZ,
		  int ncliques, const char* in);

	inline const BoardContents & emptyBoard() const { return m_emptyBoard; }

protected:
	void addClique(QVector<int> data);
	
	QVector<QVector<int> > m_cliques;

	QString m_name;
	ksudoku::GameType   m_type;
	SudokuType          m_specificType;

	int m_order;
	int m_base;
	int m_sizeX, m_sizeY, m_sizeZ;

	BoardContents m_emptyBoard;
};

#endif
