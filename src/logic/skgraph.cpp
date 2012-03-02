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
#include <QMultiMap>

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
	indexCellsToCliques();
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
	initRoxdokuGroups(0);
	indexCellsToCliques();
}

void SKGraph::initRoxdokuGroups(int pos)
{
	// initRoxdokuGroups() sets up the intersecting planes in a
	// 3-D Roxdoku grid. Its only parameter shows where in the entire
	// three-dimensional layout the grid goes.

	QVector<int> xFace, yFace, zFace;
	int x = cellPosX(pos);
	int y = cellPosY(pos);
	int z = cellPosZ(pos);

	for (uint i = 0; i < m_base; i++) {
		xFace.clear();
		yFace.clear();
		zFace.clear();
                for (int j = 0; j < m_base; j++) {
                    for (int k = 0; k < m_base; k++) {
			// Intersecting faces at relative (0,0,0), (1,1,1), etc.
			xFace << cellIndex (x + i, y + j, z + k);
			yFace << cellIndex (x + k, y + i, z + j);
			zFace << cellIndex (x + j, y + k, z + i);
		    }
		}
		addClique(xFace);
		addClique(yFace);
		addClique(zFace);
	}
}

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

void SKGraph::endCustom()
{
	indexCellsToCliques();
}

void SKGraph::indexCellsToCliques()
{
	QMultiMap<int, int> cellsToCliques;
	int nCliques = cliqueCount();
	int nCells   = size();
	for (int g = 0; g < nCliques; g++) {
	    QVector<int> cellList = clique(g);
	    for (int n = 0; n < m_order; n++) {
		cellsToCliques.insert (cellList.at (n), g);
	    }
	}

	m_cellIndex.fill   (0, nCells + 1);
	m_cellCliques.fill (0, nCliques * m_order);
	int index = 0;
	for (int cell = 0; cell < nCells; cell++) {
	    m_cellIndex [cell] = index;
	    QList<int> cliqueList = cellsToCliques.values (cell);
	    foreach (int g, cliqueList) {
		m_cellCliques [index] = g;
		index++;
	    }
	}
	m_cellIndex [nCells] = index;
}

const QList<int> SKGraph::cliqueList(int cell) const
{
	// NOTE: We could have used QMultiMap<int, int>::values(int cell) to
	// access this index, but the index's main usage is on an inner loop
	// of the generator/solver and execution time is a concern there.

	QList<int> cells;
	int start = m_cellIndex.at(cell);
	int end   = m_cellIndex.at(cell + 1);
	for (int n = start; n < end; n++) {
	    cells << m_cellCliques.at(n);
	}
	return cells;
}
