/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006      Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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
#include "skbase.h"

/**
	@author 
*/

class SKSolver;
//class glWindow;
class SKGraph  : public SKBase
{
	friend class SKSolver;

public:
	inline int sizeX() { return m_sizeX; }
	inline int sizeY() { return m_sizeY; }
	inline int sizeZ() { return m_sizeZ; }

	inline void setSizeX(int n) { m_sizeX = n; }
	inline void setSizeY(int n) { m_sizeY = n; }
	inline void setSizeZ(int n) { m_sizeZ = n; }
	inline void setSize (int n) { size = n; }
	
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
	//int optimized_sc_d;
	//int optimized_sc[25*5][625];
//	bool strongly_connected[125][625];
	int  sc_count;
	
	//unsigned int optimized_sc;

	explicit SKGraph(int o=9, bool threedimensionalf = false) : SKBase(o, threedimensionalf)
	{
		ITERATE(i,size)
		{
//			ITERATE(j,size) graph[i][j]=0; 
			optimized_d[i]=0;
		}
	}
	
public:
	/**
	 * @brief Searches for existing connections.
	 * @returns Returns true if a connection between nodes @param i and @param j exists
	 * otherwise false.
	 */
	bool hasConnection(int i, int j) const;

public:
	virtual void init() = 0;

protected:
	void addConnection(int i, int j);
	
protected:
	int m_sizeX, m_sizeY, m_sizeZ;
};

namespace ksudoku {
	
class Graph2d : public SKGraph {
public:
	explicit Graph2d(int o=9) : SKGraph(o, false) {}
	
	inline bool hasLeftBorder(int x, int y) {
		return m_borderLeft[cellIndex(x,y,0)];
	}
	inline bool hasTopBorder(int x, int y) {
		return m_borderTop[cellIndex(x,y,0)];
	}
	inline bool hasRightBorder(int x, int y) {
		return m_borderRight[cellIndex(x,y,0)];
	}
	inline bool hasBottomBorder(int x, int y) {
		return m_borderBottom[cellIndex(x,y,0)];
	}
	
	void addClique(int count, int* data);
	
protected:
	// borders are only used for 2d sudokus however QBitArrays don't require
	// much space uninitialiced
	bool m_withBorders;
	QBitArray m_borderLeft;
	QBitArray m_borderTop;
	QBitArray m_borderRight;
	QBitArray m_borderBottom;
};

class GraphSudoku : public Graph2d {
	public:
		explicit GraphSudoku(int o=9) : Graph2d(o) {}
	public:
		void init();
};

class GraphRoxdoku : public SKGraph {
	public:
		explicit GraphRoxdoku(int o=9) : SKGraph(o, true) {}
	public:
		void init();
};

class pos
{
public:
	int x, y, z;
};

class GraphCustom : public Graph2d
{
public:
	const char* filename;
	char* name;
	bool valid;

	std::vector<std::vector<int> > cliques; //or chars? don't remove SPACE
private:
// // 	int maxConnections;
// 	std::vector<int> linksUp;
// 	std::vector<int> linksLeft;//I use standard library because i want the core algorithm to be no dependent on qt/kde
public:
	//TODO use this one also in puzzle.h line 80
// 	uint get_x(int index)
// 	{
// 		int y=get_y(index);
// 		if(sizeX()==0) return 0;
// 		index-=get_z(index);
// 		index/=sizeZ();
// 		index-=y;
// 		return index/sizeY();
// 	}
// 	uint get_y(int index)
// 	{
// 		if(sizeY()==0) return 0;
// 		index-=get_z(index);
// 		index/=sizeZ();
// 		return index%sizeY();
// 	}
// 	uint get_z(int index)
// 	{
// 		if(sizeZ()==0) return 0; //TODO error
// 		return index%sizeZ();
// 	}
public:
	GraphCustom();
	explicit GraphCustom(const char* filenamed);
	static SKSolver* createCustomSolver(const char* path);
public:
	void init() {}
	void init(const char* name, int order, int sizeX, int sizeY, int sizeZ, int ncliques, const char* in);
};

}

#endif
