/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include <vector>
#include <QBitArray>
#include <QVector>

#include "ksudoku_types.h"
#include "globals.h"

/**
	@author 
*/

class ItemBoard;
class Ruleset;
class SKGraph
{
public:
	inline int sizeX() { return m_sizeX; }
	inline int sizeY() { return m_sizeY; }
	inline int sizeZ() { return m_sizeZ; }

	inline int size() { return m_sizeX * m_sizeY * m_sizeZ; }

	inline int cellIndex(uint x, uint y, uint z = 0) 
	{
		return (x*sizeY() + y)*sizeZ() + z;
	}
	inline int cellPosX(int i) {
		if(!(sizeX() && sizeY() && sizeZ())) return 0;
		return i/sizeZ()/sizeY();
	}
	inline int cellPosY(int i) {
		if(!(sizeX() && sizeY() && sizeZ())) return 0;
		return i/sizeZ()%sizeY();
	}
	inline int cellPosZ(int i) {
		if(!(sizeX() && sizeY() && sizeZ())) return 0;
		return i%sizeZ();
	}
	
	
public:
	int optimized_d[625];
	int optimized[625][625]; //pointer-style list of connected nodes

public:
	virtual ~SKGraph();
			
public:
	explicit SKGraph(int o=9, bool threedimensionalf = false);

public:
	inline int order() const { return m_order; }

public:
	/**
	 * @brief Searches for existing connections.
	 * @returns Returns true if a connection between nodes @param i and @param j exists
	 * otherwise false.
	 */
	bool hasConnection(int i, int j) const;

public:
	virtual void init() = 0;
	virtual ksudoku::GameType type() const = 0;
	virtual SudokuType specificType() const { return m_specificType; }

public:
	const Ruleset *rulset() const { return m_ruleset; }
	const ItemBoard *board() const { return m_board; }
protected:
	void addConnection(int i, int j);
	
protected:
	SudokuType m_specificType;

	int m_order;
	int m_sizeX, m_sizeY, m_sizeZ;
protected:
	Ruleset *m_ruleset;
	ItemBoard *m_board;
};

namespace ksudoku {
	
class Graph2d : public SKGraph {
public:
	explicit Graph2d(int o=9) : SKGraph(o, false) {}
	
	void addClique(QVector<int> data);
	
	int cliqueCount() { return m_cliques.count(); }

	QVector<int> clique(int i) { return m_cliques[i]; }
protected:
	QVector<QVector<int> > m_cliques;
};

class GraphSudoku : public Graph2d {
	public:
		explicit GraphSudoku(int o=9) : Graph2d(o) {}
	public:
		void init();
		ksudoku::GameType type() const { return TypeSudoku; }
};

class GraphRoxdoku : public SKGraph {
	public:
		explicit GraphRoxdoku(int o=9) : SKGraph(o, true) {}
	public:
		void init();
		ksudoku::GameType type() const { return TypeRoxdoku; }
};

class GraphCustom : public Graph2d
{
public:
	const char* filename;
	char* name;
	bool valid;

public:
	GraphCustom();
	explicit GraphCustom(const char* filenamed);
public:
	void init() {}
	ksudoku::GameType type() const { return TypeCustom; }
	void init(const char* name, SudokuType specificType, int order, int sizeX, int sizeY, int sizeZ, int ncliques, const char* in);
};

}

#endif
