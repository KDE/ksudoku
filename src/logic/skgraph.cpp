/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "skgraph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>

#include "skpuzzle.h"
#include "sksolver.h"

//SKGraph::SKGraph()
//{
//}


SKGraph::~SKGraph()
{
}

bool SKGraph::hasConnection(int i, int j) const {
	ITERATE(k,optimized_d[i])
	{
		if(optimized[i][k] == j)
			return true;
	}
	return false;
}

inline void SKGraph::addConnection(int i, int j)
{
	ITERATE(k,optimized_d[i])
	{
		if(optimized[i][k] == j)
			return;
	}
	optimized[i][optimized_d[i]++] = j;
}

void ksudoku::GraphSudoku::init()
{
	m_sizeX = order;
	m_sizeY = order;
	m_sizeZ = 1;
	
	
	
	int row, col, subsquare;
	
	ITERATE(i,size)
	{
		row       = i / order;
		col       = i % order;
		subsquare = col/base + (row/base)*base;
		
		optimized_d[i] = 0;
		
		ITERATE(j,order)
		{
			addConnection(i, j+row*order);
			addConnection(i, j*order+col);
			addConnection(i, ((subsquare/base)*base + j%base) * order + (subsquare%base)*base + j/base);
		}
	}
	
	// initialize borders
	m_borderLeft.resize(size);
	m_borderTop.resize(size);
	m_borderRight.resize(size);
	m_borderBottom.resize(size);
	for(int i = 0; i < base; ++i) {
		for(int j = 0; j < order; ++j) {
			m_borderLeft.setBit(cellIndex(i*base, j, 0));
			m_borderTop.setBit(cellIndex(j, i*base, 0));
			m_borderRight.setBit(cellIndex((i+1)*base-1, j, 0));
			m_borderBottom.setBit(cellIndex(j, (i+1)*base-1, 0));
		}
	}
}

void ksudoku::GraphRoxdoku::init()
{
	m_sizeX = base;
	m_sizeY = base;
	m_sizeZ = base;
	
	
	int faces[3];
	ITERATE(i,size)
	{
		faces[0] = i / order;
		faces[1] = i % base;
		faces[2] = (i % order) / base;
		
		optimized_d[i] = 0;
		
		ITERATE(j,size)
		{
			if(j / order == faces[0]) addConnection(i, j);
			if(j % base == faces[1]) addConnection(i, j);
			if((j % order) / base == faces[2]) addConnection(i, j);
		}
	}
	
	// no need to initialize borders, roxdokus have no borders
}

ksudoku::GraphCustom::GraphCustom(const char* filenamed)
{
	filename=filenamed; //TODO fix
	type = 2;
	order = base = size = 0;
	ITERATE(i,625)	optimized_d[i]=0;
}

ksudoku::GraphCustom::GraphCustom()
{
	type = 2;
	order = base = size = 0;
	ITERATE(i,625)	optimized_d[i]=0;

}

void ksudoku::Graph2d::addClique(int count, int* data) {
	bool hasBorders = true;
	
	// check whether this clicque needs shown borders
	bool horz = true, vert = true;
	int x = cellPosX(data[0]);
	int y = cellPosY(data[0]);
	for(int i = 1; i < count; ++i) {
		if(x != cellPosX(data[i])) vert = false;
		if(y != cellPosY(data[i])) horz = false;
	}
	if(vert || horz) hasBorders = false;
	
	// addBorders
	if(hasBorders) {
		for(int i = 0; i < count; ++i) {
			int index = data[i];
			char borders = 0;
			int posX1 = cellPosX(index);
			int posY1 = cellPosY(index);
			for(int j = 0; j < count; ++j) {
				int posX2 = cellPosX(data[j]);
				int posY2 = cellPosY(data[j]);
				if(posY1 == posY2) {
					if(posX1 == posX2 +1) borders |= 0x1;
					if(posX1 == posX2 -1) borders |= 0x2;
				}
				if(posX1 == posX2) {
					if(posY1 == posY2 +1) borders |= 0x4;
					if(posY1 == posY2 -1) borders |= 0x8;
				}
			}
			if(!(borders & 0x1)) m_borderLeft.setBit(index);
			if(!(borders & 0x2)) m_borderRight.setBit(index);
			if(!(borders & 0x4)) m_borderTop.setBit(index);
			if(!(borders & 0x8)) m_borderBottom.setBit(index);
		}
	}
	
	// add connections to the graph
	for(int i = 0; i < count; ++i) {
		for(int j = 0; j < count; ++j) {
			addConnection(data[i], data[j]);
		}
	}
	
	// add to the cliques list
}

void ksudoku::GraphCustom::init(const char* _name, int _order, int sizeX, int sizeY, int sizeZ, int ncliques, const char* in)
{//TODO free in when done
	base = 0;

	name = new char[strlen(_name)+1];
	strcpy(name, _name);

	order = _order;
	m_sizeX = sizeX;
	m_sizeY = sizeY;
	m_sizeZ = sizeZ;
	size = m_sizeX*m_sizeY*m_sizeZ;

	if(order<10) zerochar='0';
	else zerochar='a'-1;

// 	linksUp   = std::vector<int>(size);
// 	linksLeft = std::vector<int>(size);

// 	for(int i=0; i<size; i++)
// 	{
// 		linksUp[i]=linksLeft[i]=0;
// 	}

// 	int max=0;
// 	maxConnections=0;
	std::istringstream is(in);

	m_borderLeft.resize(size);
	m_borderTop.resize(size);
	m_borderRight.resize(size);
	m_borderBottom.resize(size);
	
	ITERATE(i, ncliques)
	{
		
		cliques.push_back(std::vector<int>());
		//read clique line
		int n;
		is >> n;
		if(n>625) return;

		int temp[625];
		is >> temp[0];
		
// 		bool horz = true;
// 		bool vert = true;
// 		int x = cellPosX(temp[0]);
// 		int y = cellPosY(temp[0]);
		for(int j = 1; j < n; ++j) {
			is >> temp[j];
// 			if(x != cellPosX(temp[j])) vert = false;
// 			if(y != cellPosY(temp[j])) horz = false;
		}
		
		addClique(n, temp);
		
// 		if(!(horz || vert)) {
// 			for(int j = 0; j < n; ++j) {
// 				bool left = true;
// 				bool right = true;
// 				bool top = true;
// 				bool bottom = true;
// 				for(int k = 0; k < n; ++k) {
// 					if(cellPosY(temp[j]) == cellPosY(temp[k])) {
// 						if(cellPosX(temp[j]) == cellPosX(temp[k]) +1)
// 							left = false;
// 						if(cellPosX(temp[j]) == cellPosX(temp[k]) -1)
// 							right = false;
// 					}
// 					if(cellPosX(temp[j]) == cellPosX(temp[k])) {
// 						if(cellPosY(temp[j]) == cellPosY(temp[k]) +1)
// 							top = false;
// 						if(cellPosY(temp[j]) == cellPosY(temp[k]) -1)
// 							bottom = false;
// 					}
// // 					if(cellPosY(temp[j]) == cellPosY(temp[k]) +1)
// // 						top == false;;
// 				}
// 				if(left) m_borderLeft.setBit(temp[j]);
// 				if(right) m_borderRight.setBit(temp[j]);
// 				if(top) m_borderTop.setBit(temp[j]);
// 				if(bottom) m_borderBottom.setBit(temp[j]);
// 			}
// 		}

		ITERATE(j,n)
		{
			int vv;
// 			is >> vv; //TODO check errors
			vv = temp[j];

			std::vector<int>& clique = cliques.back();
			clique.push_back(vv);
// 			if(vv>max) max=vv; 

// 			ITERATE(k,j)
// 			{
// 				addConnection(clique[j], clique[k]);
// 				addConnection(clique[k], clique[j]);
				
// 				int jx,jy,kx,ky;
// 				jx = cellPosX( clique[j] );
// 				jy = cellPosY( clique[j] );
// 				kx = cellPosX( clique[k] );
// 				ky = cellPosY( clique[k] );

// 				if(jx==kx && jy==ky+1) linksLeft[ clique[j] ]++;
// 				if(jx==kx && jy==ky-1) linksLeft[ clique[k] ]++;
// 				if(jy==ky && jx==kx-1) linksUp  [ clique[k] ]++;
// 				if(jy==ky && jx==kx+1) linksUp  [ clique[j] ]++;
				
				//diagonal links -> disabled
				//if(jx==kx+1 && jy==ky+1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx-1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx+1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx-1 && jy==ky+1) {linksLeft[ clique[k] ]++; linksUp  [ clique[k] ]++;}

// 				if(linksUp  [ clique[k] ] > maxConnections) maxConnections = linksUp  [ clique[k] ];
// 				if(linksLeft[ clique[k] ] > maxConnections) maxConnections = linksLeft[ clique[k] ];
// 				if(linksUp  [ clique[j] ] > maxConnections) maxConnections = linksUp  [ clique[j] ];
// 				if(linksLeft[ clique[j] ] > maxConnections) maxConnections = linksLeft[ clique[j] ];
// 			}
		}
		
	}
	
	
// 	// setup Borders
// 	int connections;
// 	m_borderLeft.resize(size);
// 	m_borderTop.resize(size);
// 	for(int i = 0; i < size; ++i) {
// 		int x = cellPosX(i);
// 		int y = cellPosY(i);
// 		if(y > 0 && linksLeft[i]) {
// 			connections = 3 - linksLeft[i];
// 			if(connections >= 2) m_borderLeft.setBit(i);
// 		}
// 		if(x > 0 && linksUp[i]) {
// 			connections = 3 - linksUp[i];
// 			if(connections >= 2) m_borderTop.setBit(i);
// 		}
// 	}
	
	//printf("%d\n", size);
	valid=true;
	return;
}

// void ksudoku::GraphCustom::init()
// {
// 	if(filename==0) return;
// 	order=base=0;
// 	FILE* in = fopen(filename, "r");
// 	if(in == NULL) return;
// 	if(fscanf(in, "%d\n", &order) == EOF) return;
// 	if(order==0)return;
// 	if(order<10) zerochar='0';
// 	else zerochar='a'-1;
// 
// 	if(fscanf(in, "%d %d %d\n", &m_sizeX, &m_sizeY, &m_sizeZ) == EOF) return;//Z for further compatibility
// 	if(m_sizeX<1 || m_sizeY<1 || m_sizeZ<1) return;
// 	size = m_sizeX*m_sizeY*m_sizeZ;
// 
// 	linksUp   = std::vector<int>(size);
// 	linksLeft = std::vector<int>(size);
// 
// 	for(int i=0; i<size; i++)
// 	{
// 		linksUp[i]=linksLeft[i]=0;
// 	}
// 
// 	//READ CLIQUES
// 	int ncliques=0;
// 
// 	if(fscanf(in, "%d\n", &ncliques) == EOF) return;
// 	if(ncliques==0)return;
// 	int max=0;
// 	maxConnections=0;
// 
// 	ITERATE(i, ncliques)
// 	{
// 		cliques.push_back(std::vector<int>());
// 		//read clique line
// 		int n;
// 		if(fscanf(in, "%d ", &n)==EOF) return;
// 		if(n>625) return;
// 
// 		ITERATE(j,n)
// 		{
// 			int vv;
// 			if(fscanf(in, "%d", &vv) == EOF) return;
// 			std::vector<int>& clique = cliques.back();
// 			clique.push_back(vv);
// 			if(vv>max) max=vv; 
// 
// 			ITERATE(k,j)
// 			{
// 				addConnection(clique[j], clique[k]);
// 				addConnection(clique[k], clique[j]);
// 				
// 				int jx,jy,kx,ky;
// 				jx = cellPosX( clique[j] );
// 				jy = cellPosY( clique[j] );
// 				kx = cellPosX( clique[k] );
// 				ky = cellPosY( clique[k] );
// 
// 				if(jx==kx && jy==ky+1) linksLeft[ clique[j] ]++;
// 				if(jx==kx && jy==ky-1) linksLeft[ clique[k] ]++;
// 				if(jy==ky && jx==kx-1) linksUp  [ clique[k] ]++;
// 				if(jy==ky && jx==kx+1) linksUp  [ clique[j] ]++;
// 				
// 				//diagonal links -> disabled
// 				//if(jx==kx+1 && jy==ky+1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
// 				//if(jx==kx-1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
// 				//if(jx==kx+1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
// 				//if(jx==kx-1 && jy==ky+1) {linksLeft[ clique[k] ]++; linksUp  [ clique[k] ]++;}
// 
// 				if(linksUp  [ clique[k] ] > maxConnections) maxConnections = linksUp  [ clique[k] ];
// 				if(linksLeft[ clique[k] ] > maxConnections) maxConnections = linksLeft[ clique[k] ];
// 				if(linksUp  [ clique[j] ] > maxConnections) maxConnections = linksUp  [ clique[j] ];
// 				if(linksLeft[ clique[j] ] > maxConnections) maxConnections = linksLeft[ clique[j] ];
// 			}
// 		}
// 		
// 	}
// 	//printf("%d\n", size);
// 	
// 	// setup Borders
// 	int connections;
// 	m_borderLeft.resize(size);
// 	m_borderTop.resize(size);
// 	for(int i = 0; i < size; ++i) {
// 		int x = cellPosX(i);
// 		int y = cellPosY(i);
// 		if(y > 0 && linksLeft[i]) {
// 			connections = 3-linksLeft[i];
// 			if(connections >= 2) m_borderLeft.setBit(i);
// 		}
// 		if(x > 0 && linksUp[i]) {
// 			connections = 3-linksUp[i];
// 			if(connections >= 2) m_borderTop.setBit(i);
// 		}
// 	}
// 	
// 	valid=true;
// 	fclose(in);
// 	return;
// }
// 
// SKSolver* ksudoku::GraphCustom::createCustomSolver(const char* path)
// {
// 	//only 4 testing: buggy
// 	ksudoku::GraphCustom* gc = new ksudoku::GraphCustom(path);
// 	gc->init();
// 	if(gc->valid==false) return NULL;
// 	
// 	SKSolver* solver = new SKSolver(gc);
// 	
// 	//SKPuzzle* puzzle    = new SKPuzzle();
// 	//puzzle->size=solver->size;
// 	solver->setType(custom);
// 	return solver;
// }
//only for testing !!! !!! !!!
/*int main(void)
{
	int a = time(0);
	std::srand(a);
	printf("%d\n",a);
	
	ksudoku::GraphCustom* gc = new ksudoku::GraphCustom("test_custompuzzle.txt");
	gc->init();
	
	SKSolver* solver = new SKSolver(gc);
	SKPuzzle* puzzle    = new SKPuzzle();
	puzzle->order=4;
	puzzle->size=solver->size;

	solver->solve(puzzle, 1, puzzle);	

	for(int i=0; i<gc->size; i++)
	{
		for(int j=0; j<gc->size; j++)
		{
			printf("%.3d ", gc->optimized[i][j]);
		}
		printf("\n");
	}

	for(int i=0; i<16; i++)
	{
		if(i%4==0) printf("\n");
		printf("%d ", puzzle->numbers[i]);
	}
	printf("\n");
	
	
	return 0;
}*/
