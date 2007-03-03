//
// C++ Implementation: skgraph
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
void ksudoku::GraphCustom::init(const char* _name, int _order, int sizeX, int sizeY, int sizeZ, int ncliques, const char* in)
{//TODO free in when done
	base = 0;

	name = new char[strlen(_name)];
	strcpy(name, _name);

	order = _order;
	m_sizeX = sizeX;
	m_sizeY = sizeY;
	m_sizeZ = sizeZ;
	size = m_sizeX*m_sizeY*m_sizeZ;

	if(order<10) zerochar='0';
	else zerochar='a'-1;

	linksUp   = std::vector<int>(size);
	linksLeft = std::vector<int>(size);

	for(int i=0; i<size; i++)
	{
		linksUp[i]=linksLeft[i]=0;
	}

	int max=0;
	maxConnections=0;
	std::istringstream is(in);

	ITERATE(i, ncliques)
	{
		cliques.push_back(std::vector<int>());
		//read clique line
		int n;
		is >> n;
		if(n>625) return;

		ITERATE(j,n)
		{
			int vv;
			is >> vv; //TODO check errors

			std::vector<int>& clique = cliques.back();
			clique.push_back(vv);
			if(vv>max) max=vv; 

			ITERATE(k,j)
			{
				addConnection(clique[j], clique[k]);
				addConnection(clique[k], clique[j]);
				
				int jx,jy,kx,ky;
				jx = get_x( clique[j] );
				jy = get_y( clique[j] );
				kx = get_x( clique[k] );
				ky = get_y( clique[k] );

				if(jx==kx && jy==ky+1) linksLeft[ clique[j] ]++;
				if(jx==kx && jy==ky-1) linksLeft[ clique[k] ]++;
				if(jy==ky && jx==kx-1) linksUp  [ clique[k] ]++;
				if(jy==ky && jx==kx+1) linksUp  [ clique[j] ]++;
				
				//diagonal links -> disabled
				//if(jx==kx+1 && jy==ky+1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx-1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx+1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx-1 && jy==ky+1) {linksLeft[ clique[k] ]++; linksUp  [ clique[k] ]++;}

				if(linksUp  [ clique[k] ] > maxConnections) maxConnections = linksUp  [ clique[k] ];
				if(linksLeft[ clique[k] ] > maxConnections) maxConnections = linksLeft[ clique[k] ];
				if(linksUp  [ clique[j] ] > maxConnections) maxConnections = linksUp  [ clique[j] ];
				if(linksLeft[ clique[j] ] > maxConnections) maxConnections = linksLeft[ clique[j] ];
			}
		}
		
	}
	//printf("%d\n", size);
	valid=true;
	return;
}

void ksudoku::GraphCustom::init()
{
	if(filename==0) return;
	order=base=0;
	FILE* in = fopen(filename, "r");
	if(in == NULL) return;
	if(fscanf(in, "%d\n", &order) == EOF) return;
	if(order==0)return;
	if(order<10) zerochar='0';
	else zerochar='a'-1;

	if(fscanf(in, "%d %d %d\n", &m_sizeX, &m_sizeY, &m_sizeZ) == EOF) return;//Z for further compatibility
	if(m_sizeX<1 || m_sizeY<1 || m_sizeZ<1) return;
	size = m_sizeX*m_sizeY*m_sizeZ;

	linksUp   = std::vector<int>(size);
	linksLeft = std::vector<int>(size);

	for(int i=0; i<size; i++)
	{
		linksUp[i]=linksLeft[i]=0;
	}

	//READ CLIQUES
	int ncliques=0;

	if(fscanf(in, "%d\n", &ncliques) == EOF) return;
	if(ncliques==0)return;
	int max=0;
	maxConnections=0;

	ITERATE(i, ncliques)
	{
		cliques.push_back(std::vector<int>());
		//read clique line
		int n;
		if(fscanf(in, "%d ", &n)==EOF) return;
		if(n>625) return;

		ITERATE(j,n)
		{
			int vv;
			if(fscanf(in, "%d", &vv) == EOF) return;
			std::vector<int>& clique = cliques.back();
			clique.push_back(vv);
			if(vv>max) max=vv; 

			ITERATE(k,j)
			{
				addConnection(clique[j], clique[k]);
				addConnection(clique[k], clique[j]);
				
				int jx,jy,kx,ky;
				jx = get_x( clique[j] );
				jy = get_y( clique[j] );
				kx = get_x( clique[k] );
				ky = get_y( clique[k] );

				if(jx==kx && jy==ky+1) linksLeft[ clique[j] ]++;
				if(jx==kx && jy==ky-1) linksLeft[ clique[k] ]++;
				if(jy==ky && jx==kx-1) linksUp  [ clique[k] ]++;
				if(jy==ky && jx==kx+1) linksUp  [ clique[j] ]++;
				
				//diagonal links -> disabled
				//if(jx==kx+1 && jy==ky+1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx-1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx+1 && jy==ky-1) {linksLeft[ clique[j] ]++; linksUp  [ clique[j] ]++;}
				//if(jx==kx-1 && jy==ky+1) {linksLeft[ clique[k] ]++; linksUp  [ clique[k] ]++;}

				if(linksUp  [ clique[k] ] > maxConnections) maxConnections = linksUp  [ clique[k] ];
				if(linksLeft[ clique[k] ] > maxConnections) maxConnections = linksLeft[ clique[k] ];
				if(linksUp  [ clique[j] ] > maxConnections) maxConnections = linksUp  [ clique[j] ];
				if(linksLeft[ clique[j] ] > maxConnections) maxConnections = linksLeft[ clique[j] ];
			}
		}
		
	}
	//printf("%d\n", size);
	valid=true;
	fclose(in);
	return;
}

SKSolver* ksudoku::GraphCustom::createCustomSolver(const char* path)
{
	//only 4 testing: buggy
	ksudoku::GraphCustom* gc = new ksudoku::GraphCustom(path);
	gc->init();
	if(gc->valid==false) return NULL;
	
	SKSolver* solver = new SKSolver(gc);
	
	//SKPuzzle* puzzle    = new SKPuzzle();
	//puzzle->size=solver->size;
	solver->setType(custom);
	return solver;
}
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
