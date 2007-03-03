//
// C++ Interface: skgraph
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SKGRAPH_H
#define SKGRAPH_H

#include <vector>
#include "skbase.h"

/**
	@author 
*/

namespace ksudoku {
	class Game;
}
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

	SKGraph(int o=9, bool threedimensionalf = false) : SKBase(o, threedimensionalf)
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

class GraphSudoku : public SKGraph {
	public:
		GraphSudoku(int o=9) : SKGraph(o, false) {}
	public:
		void init();
};

class GraphRoxdoku : public SKGraph {
	public:
		GraphRoxdoku(int o=9) : SKGraph(o, true) {}
	public:
		void init();
};

class pos
{
public:
	int x, y, z;
};

class GraphCustom : public SKGraph
{
public:
	const char* filename;
	char* name;
	bool valid;
	int maxConnections;

	std::vector<std::vector<int> > cliques; //or chars? dont remove SPACE
	std::vector<int> linksUp;
	std::vector<int> linksLeft;//I use standard library because i want the core algorithm to be no dependent on qt/kde
public:
	//TODO use this one also in puzzle.h line 80
	uint index(uint x, uint y, uint z = 0) 
	{
		return (x*sizeY() + y)*sizeZ() + z;
	}
	uint get_x(int index)
	{
		int y=get_y(index);
		if(sizeX()==0) return 0;
		index-=get_z(index);
		index/=sizeZ();
		index-=y;
		return index/sizeY();
	}
	uint get_y(int index)
	{
		if(sizeY()==0) return 0;
		index-=get_z(index);
		index/=sizeZ();
		return index%sizeY();
	}
	uint get_z(int index)
	{
		if(sizeZ()==0) return 0; //TODO error
		return index%sizeZ();
	}
public:
	GraphCustom();
	GraphCustom(const char* filenamed);
	static SKSolver* createCustomSolver(const char* path);
public:
	void init();
	void init(const char* name, int order, int sizeX, int sizeY, int sizeZ, int ncliques, const char* in);
};

}

#endif
