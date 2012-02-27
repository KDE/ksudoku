/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
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

#include "skgraph.h"

#include <QString>
#include <QStringList>

SKGraph::SKGraph(int order, ksudoku::GameType type)
{
	m_order = order;
	m_base  = 3;
	for (int n = 2; n <= 5; n++) {
	    if (m_order == n * n) {
		m_base = n;
	    }
	}
	m_type = type;
}

SKGraph::~SKGraph()
{
}

void SKGraph::initSudoku()
{
	m_name = "PlainSudoku";
	m_specificType = Plain;

	m_sizeX = m_order;
	m_sizeY = m_order;
	m_sizeZ = 1;
	m_emptyBoard.fill (UNUSABLE, size());
	initSudokuGroups(0, true);
}

void SKGraph::initSudokuGroups(int pos, bool withBlocks)
{
	// initSudokuGroups() sets up rows and columns in a Sudoku grid of size
	// (m_order*m_order) cells. Its first parameter (usually 0) shows where
	// on the whole board the grid goes. This is relevant in Samurai and
	// related layouts. Its second attribute is true if square-block groups
	// are required or false if not (e.g. as in a Jigsaw type).

	QVector<int> rowc, colc, blockc;
	for(int i = 0; i < m_order; ++i) {
		rowc.clear();
		colc.clear();
		blockc.clear();
		
		for(int j = 0; j < m_order; ++j)
		{
			rowc   << pos + j*m_sizeY + i;
			colc   << pos + i*m_sizeY + j;
			blockc << pos + ((i/m_base)*m_base + j%m_base) * m_sizeY				      + (i%m_base)*m_base + j/m_base;
		}
		addClique(rowc);
		addClique(colc);
		if (withBlocks) {
			addClique(blockc);
		}
	}
}

void SKGraph::initRoxdoku()
{
	m_name = "Roxdoku";
	m_specificType = Roxdoku;

	m_sizeX = m_base;
	m_sizeY = m_base;
	m_sizeZ = m_base;
	m_emptyBoard.fill (UNUSABLE, size());

	QVector<int> xFace, yFace, zFace;

	for (uint i = 0; i < m_base; i++) {
		xFace.clear();
		yFace.clear();
		zFace.clear();
                for (int j = 0; j < m_base; j++) {
                    for (int k = 0; k < m_base; k++) {
			// Intersecting faces at (0,0,0), (1,1,1), (2,2,2), etc.
			xFace << cellIndex (i, j, k);
			yFace << cellIndex (k, i, j);
			zFace << cellIndex (j, k, i);
		    }
		}
		addClique(xFace);
		addClique(yFace);
		addClique(zFace);
	}
}

	// initRoxdokuGroups() sets up the intersecting planes in a
	// 3-D Roxdoku grid. Its only attribute shows where in the entire
	// three-dimensional layout the grid goes. NOT IMPLEMENTED YET.

void SKGraph::addClique(QVector<int> data) {
	// Add to the cliques (groups) list.
	m_cliques << data;
	for (int n = 0; n < data.size(); n++) {
	    // Set cells in groups VACANT: cells not in groups are UNUSABLE.
	    m_emptyBoard [data.at(n)] = VACANT;
	}
}

void SKGraph::initCustom(const QString & name, SudokuType specificType,
				int order, int sizeX, int sizeY, int sizeZ,
				int ncliques) {
	// The Serializer's deserializer methods will add groups (or cliques).
	m_name = name;
	m_specificType = specificType;

	m_order = order;
	m_sizeX = sizeX;
	m_sizeY = sizeY;
	m_sizeZ = sizeZ;
	m_emptyBoard.fill (UNUSABLE, size());
}
