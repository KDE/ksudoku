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
#include <string>
#include <iostream>
#include <sstream>

#include "sksolver.h"

#include <ruleset.h>
#include <choiceitem.h>
#include <sudokuconstraint.h>


SKGraph::SKGraph(int o, bool threedimensionalf)
{
	// <<< implementation of SKBase::setorder
	oldtype = threedimensionalf;
	m_order = o;
	int base = (int) sqrt((double)o);

	int size = (oldtype==1) ?  base*base*base :  (m_order*m_order) ;
	// >>>

	m_ruleset = 0;
	m_board = 0;
	for(int i = 0; i < size; ++i)
	{
		optimized_d[i]=0;
	}
}

SKGraph::~SKGraph()
{
	delete m_ruleset;
}

bool SKGraph::hasConnection(int i, int j) const {
	for(int k = 0; k < optimized_d[i]; ++k)
	{
		if(optimized[i][k] == j)
			return true;
	}
	return false;
}

inline void SKGraph::addConnection(int i, int j)
{
	for(int k = 0; k < optimized_d[i]; ++k)
	{
		if(optimized[i][k] == j)
			return;
	}
	optimized[i][optimized_d[i]++] = j;
}

void ksudoku::GraphSudoku::init()
{
	int base = static_cast<int>(sqrt((float)m_order));
	
	m_ruleset = new Ruleset();
	ItemBoard *board = new ItemBoard(m_order, m_order);
	m_ruleset->addItem(board);
	m_board = board;
	
	for(int x = 0; x < m_order; ++x) {
		for(int y = 0; y < m_order; ++y) {
			board->setItem(new ChoiceItem(1, m_order), x, y);
		}
	}
	board->init(m_ruleset);

	for(int i = 0; i < m_order; ++i) {
		SudokuConstraint *row = new SudokuConstraint();
		QVector<Item*> items = board->itemsAt(-1, i);
		row->setItems(items);
		row->init(m_ruleset);
		m_ruleset->addItem(row);

		SudokuConstraint *column = new SudokuConstraint();
		column->setItems(board->itemsAt(i, -1));
		column->init(m_ruleset);
		m_ruleset->addItem(column);

		SudokuConstraint *block = new SudokuConstraint();
		int blockRow = i/base*base;
		int blockCol = i%base*base;
		for(int ibh = 0; ibh < base; ++ibh) {
			for(int ibw = 0; ibw < base; ++ibw) {
				block->addItem(board->itemAt(blockCol + ibw, blockRow + ibh));
			}
		}
		block->init(m_ruleset);
		m_ruleset->addItem(block);
	}
	// Finished initialization of ruleset

	m_sizeX = m_order;
	m_sizeY = m_order;
	m_sizeZ = 1;

	int row, col, subsquare;

	for(int i = 0; i < size(); ++i) {
		optimized_d[i] = 0;
	}

	
	QVector<int> rowc, colc, blockc;
	for(int i = 0; i < m_order; ++i) {
		rowc.clear();
		colc.clear();
		blockc.clear();
		
		for(int j = 0; j < m_order; ++j)
		{
			rowc << j+i*m_order;
			colc << j*m_order+i;
			blockc << ((i/base)*base + j%base) * m_order + (i%base)*base + j/base;
		}
		addClique(rowc);
		addClique(colc);
		addClique(blockc);
	}
}

void ksudoku::GraphRoxdoku::init()
{
	int base = static_cast<int>(sqrt((float)m_order));
	
	m_ruleset = new Ruleset();
	ItemBoard *board = new ItemBoard(base, base, base);
	m_ruleset->addItem(board);
	m_board = board;
	
	for(int x = 0; x < base; ++x) {
		for(int y = 0; y < base; ++y) {
			for(int z = 0; z < base; ++z) {
				board->setItem(new ChoiceItem(1, m_order), x, y, z);
			}
		}
	}
	board->init(m_ruleset);
	
	for(int i = 0; i < base; ++i) {
		SudokuConstraint *constraint = new SudokuConstraint();
		QVector<Item*> items = board->itemsAt(-1, -1, i);
		constraint->setItems(items);
		constraint->init(m_ruleset);
		m_ruleset->addItem(constraint);

		SudokuConstraint *constraint2 = new SudokuConstraint();
		constraint2->setItems(board->itemsAt(-1, i, -1));
		constraint2->init(m_ruleset);
		m_ruleset->addItem(constraint2);

		SudokuConstraint *constraint3 = new SudokuConstraint();
		constraint3->setItems(board->itemsAt(i, -1, -1));
		constraint3->init(m_ruleset);
		m_ruleset->addItem(constraint3);
	}
	// Finished initialization of ruleset
	
	
	m_sizeX = base;
	m_sizeY = base;
	m_sizeZ = base;
	
	
	int faces[3];
	for(int i = 0; i < size(); ++i)
	{
		faces[0] = i / m_order;
		faces[1] = i % base;
		faces[2] = (i % m_order) / base;
		
		optimized_d[i] = 0;
		
		for(int j = 0; j < size(); ++j)
		{
			if(j / m_order == faces[0]) addConnection(i, j);
			if(j % base == faces[1]) addConnection(i, j);
			if((j % m_order) / base == faces[2]) addConnection(i, j);
		}
	}
}

ksudoku::GraphCustom::GraphCustom(const char* filenamed)
{
	filename=filenamed; //TODO fix
	oldtype = 2;
	m_order = 0;
	for(int i = 0; i < 625; ++i)	optimized_d[i]=0;
}

ksudoku::GraphCustom::GraphCustom()
{
	oldtype = 2;
	m_order = 0;
	for(int i = 0; i < 625; ++i)	optimized_d[i]=0;

}

void ksudoku::Graph2d::addClique(QVector<int> data) {
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
	name = new char[strlen(_name)+1];
	strcpy(name, _name);

	m_order = _order;
	m_sizeX = sizeX;
	m_sizeY = sizeY;
	m_sizeZ = sizeZ;

	m_ruleset = new Ruleset();
	ItemBoard *board = new ItemBoard(m_sizeX, m_sizeY, m_sizeZ);
	m_ruleset->addItem(board);
	m_board = board;

	std::istringstream is(in);

	QVector<int> data;
	for(int i = 0; i < ncliques; ++i)
	{
		data.clear();

		SudokuConstraint *constraint = new SudokuConstraint();

		//read clique line
		int n;
		is >> n;
		for(; n > 0; --n) {
			int temp;
			is >> temp;
			data << temp;

			int x = cellPosX(temp), y = cellPosY(temp), z = cellPosZ(temp);
			Item *item = board->itemAt(x, y, z);
			if(!item) {
				item = new ChoiceItem(1, m_order);
				board->setItem(item, x, y, z);
			}
			constraint->addItem(item);
		}

		constraint->init(m_ruleset);
		m_ruleset->addItem(constraint);

		addClique(data);
	}

	board->init(m_ruleset);

	
	valid=true;
	return;
}
