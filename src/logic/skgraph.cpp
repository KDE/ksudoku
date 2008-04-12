/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
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

#include "skgraph.h"
// #include <stdio.h>
// #include <stdlib.h>
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
	
	m_borderLeft.resize(size);
	m_borderTop.resize(size);
	m_borderRight.resize(size);
	m_borderBottom.resize(size);
	m_borderFront.resize(size);
	m_borderBack.resize(size);
	
	int row, col, subsquare;

	ITERATE(i,size) {
		optimized_d[i] = 0;
	}

	
	QVector<int> rowc, colc, blockc;
	ITERATE(i,order) {
		rowc.clear();
		colc.clear();
		blockc.clear();
		
		ITERATE(j,order)
		{
			rowc << j+i*order;
			colc << j*order+i;
			blockc << ((i/base)*base + j%base) * order + (i%base)*base + j/base;
		}
		addClique(rowc);
		addClique(colc);
		addClique(blockc);
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

void ksudoku::Graph2d::addClique(QVector<int> data) {
	bool hasBorders = true;
	
	// check whether this clicque needs shown borders
	bool horz = true, vert = true;
	int x = cellPosX(data[0]);
	int y = cellPosY(data[0]);
	for(int i = 1; i < data.size(); ++i) {
		if(x != cellPosX(data[i])) vert = false;
		if(y != cellPosY(data[i])) horz = false;
	}
	if(vert || horz) hasBorders = false;
	
	// addBorders
	if(hasBorders) {
		for(int i = 0; i < data.size(); ++i) {
			int index = data[i];
			char borders = 0;
			int posX1 = cellPosX(index);
			int posY1 = cellPosY(index);
			int posZ1 = cellPosZ(index);
			for(int j = 0; j < data.size(); ++j) {
				int posX2 = cellPosX(data[j]);
				int posY2 = cellPosY(data[j]);
				int posZ2 = cellPosZ(data[j]);
				if(posY1 == posY2 && posZ1 == posZ2) {
					if(posX1 == posX2 +1) borders |= 0x01;
					if(posX1 == posX2 -1) borders |= 0x02;
				}
				if(posX1 == posX2 && posZ1 == posZ2) {
					if(posY1 == posY2 +1) borders |= 0x04;
					if(posY1 == posY2 -1) borders |= 0x08;
				}
				if(posX1 == posX2 && posY1 == posY2) {
					if(posX1 == posX2 +1) borders |= 0x10;
					if(posX1 == posX2 -1) borders |= 0x20;
				}
			}
			if(!(borders & 0x01)) m_borderLeft.setBit(index);
			if(!(borders & 0x02)) m_borderRight.setBit(index);
			if(!(borders & 0x04)) m_borderTop.setBit(index);
			if(!(borders & 0x08)) m_borderBottom.setBit(index);
			if(!(borders & 0x10)) m_borderFront.setBit(index);
			if(!(borders & 0x20)) m_borderBack.setBit(index);
		}
	}
	
	// add connections to the graph
	for(int i = 0; i < data.size(); ++i) {
		for(int j = 0; j < data.size(); ++j) {
			addConnection(data[i], data[j]);
		}
	}
	
	// add to the cliques list
	m_cliques << data;
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
	m_borderFront.resize(size);
	m_borderBack.resize(size);
	
	QVector<int> data;
	ITERATE(i, ncliques)
	{
		data.clear();
		//read clique line
		int n;
		is >> n;
		for(; n > 0; --n) {
			int temp;
			is >> temp;
			data << temp;
		}
		
		addClique(data);
	}
	
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
