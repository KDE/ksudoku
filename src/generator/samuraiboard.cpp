/****************************************************************************
 *    Copyright 2011  Ian Wadham <iandw.au@gmail.com>                       *
 *                                                                          *
 *    This program is free software; you can redistribute it and/or         *
 *    modify it under the terms of the GNU General Public License as        *
 *    published by the Free Software Foundation; either version 2 of        *
 *    the License, or (at your option) any later version.                   *
 *                                                                          *
 *    This program is distributed in the hope that it will be useful,       *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *    GNU General Public License for more details.                          *
 *                                                                          *
 *    You should have received a copy of the GNU General Public License     *
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ****************************************************************************/

#include "debug.h"

#include "samuraiboard.h"

#include <stdio.h>

SamuraiBoard::SamuraiBoard (QObject * parent,
                            SudokuType sudokuType, int blockSize)
    :
    PlainSudokuBoard     (parent, sudokuType, blockSize)
{
    dbgLevel = 1;

    m_overlap = blockSize;
}	

void SamuraiBoard::setUpBoard()
{
    dbo "Samurai setup: Block %d, order %d, overlap %d, type %d\n",
            m_blockSize, m_order, m_overlap, m_type);
    m_boardSize = 3 * m_order - 2 * m_overlap;
    m_boardArea = m_boardSize * m_boardSize;

    // Create an empty board of the required size.
    clear (m_currentValues);

    m_nGroups = 5 * 2 * m_order;	// Rows and columns in 5 grids.
    int blocks = (m_overlap == m_blockSize) ? 5 * m_order - 4 : 5 * m_order;
    m_nGroups = m_nGroups + blocks;	// Add in blocks.
    m_groupSize = m_order;
    m_groupList.fill (0, m_nGroups * m_groupSize);

    // Make row and column indexes for the left, centre and right-hand grids.
    qint32 index = 0;
    index = makeRowColIndex (0, 0, index);
    index = makeRowColIndex (0, m_boardSize - m_order, index);
    index = makeRowColIndex (m_order - m_overlap,
                             m_order - m_overlap, index);
    index = makeRowColIndex (m_boardSize - m_order, 0, index);
    index = makeRowColIndex (m_boardSize - m_order,
                             m_boardSize - m_order, index);
    index = makeBlockIndex  (index);
    dbo1 "The index reached %d.\n", index);

    // Make the solver faster (see SudokuBoard::updateValueRequirements()).
    indexCellsToGroups();

    dbo1 "Block %dx%d = %d, grid %dx%d = %d, board %dx%d = %d\n",
        m_blockSize, m_blockSize, m_blockSize * m_blockSize,
        m_order, m_order, m_gridArea,
        m_boardSize, m_boardSize, m_boardArea);
    dbo1 "There are %d groups of %d cells each.\n", m_nGroups, m_groupSize);
}

void SamuraiBoard::clear (BoardContents & boardValues)
{
    // Create an empty board of the required size.
    boardValues.fill (0, m_boardArea);

    // Set unusable cells to -1 (m_unusable).
    markUnusable (boardValues,				// Middle top.
                  0, m_order - m_overlap,
                  m_order, m_boardSize - m_order);
    markUnusable (boardValues,				// Middle bottom.
                  m_boardSize - m_order + m_overlap, m_boardSize,
                  m_order, m_boardSize - m_order);
    markUnusable (boardValues,				// Middle left.
                  m_order, m_boardSize - m_order,
                  0, m_order - m_overlap);
    markUnusable (boardValues,				// Middle right.
                  m_order, m_boardSize - m_order,
                  m_boardSize - m_order + m_overlap, m_boardSize);
}

BoardContents & SamuraiBoard::fillBoard()
{
    // A Samurai board has five overlapping grids of the Plain Sudoku type.
    // One of these grids is filled with values at random, then the other four
    // grids are filled, but values from an overlapping area are preset.

    const int nGrids            = 5;
    const int mid               = m_order - m_overlap;
    const int low               = 2 * m_order - 2 * m_overlap;
    const int w                 = m_order;
    const int h                 = m_order;
    const int gridRow [nGrids]  = {mid, 0,   0, low, low};
    const int gridCol [nGrids]  = {mid, 0, low,   0, low};

    dbo "FILL SAMURAI BOARD: mid %d, low %d, m_boardSize %d\n",
            mid, low, m_boardSize);
    clear (m_currentValues);

    // Create a Plain Sudoku board and fill it with values.
    PlainSudokuBoard * grid = new PlainSudokuBoard (this, Plain, m_blockSize);
    BoardContents gridValues;
    grid->setUpBoard();
    dbo1 "FIRST GRID HAS BEEN SET UP.\n");
    gridValues = grid->fillBoard();

    // Copy the first set of values into the Samurai board.
    int n = 0;
    int base = gridRow [n] * m_boardSize + gridCol [n];
    dbo1 "FIRST GRID IS %d, row %d, col %d, base %d\n",
            n, gridRow [n], gridCol [n], base);
    copyCells (w, h, gridValues, 0, w, m_currentValues, base, m_boardSize);
    grid->print (gridValues);
    print (m_currentValues);

    // Fill more Plain Sudoku boards with values, except for the overlap area.
    int done = n;
    for (n = 0; n < nGrids; n++) {
        if (n == done) {
            continue;
        }
        grid->clear (gridValues);
        base = gridRow [n] * m_boardSize + gridCol [n];
        dbo1 "NEXT GRID IS %d, row %d, col %d, base %d\n",
                n, gridRow [n], gridCol [n], base);

        // Copy an empty area from the Samurai board to the Plain board, but
        // including an overlapping corner that has been previously filled.
        copyCells (w, h, m_currentValues, base, m_boardSize, gridValues, 0, w);
        grid->print (gridValues);

        // Fill the Plain board, starting from the overlapped values.
        gridValues = grid->solveBoard (gridValues);

        // Copy the filled Plain board back into the Samurai board.
        copyCells (w, h, gridValues, 0, w, m_currentValues, base, m_boardSize);
        grid->print (gridValues);
        print (m_currentValues);
    }
    delete grid;
    dbo "SAMURAI BOARD FILLED\n");
    return m_currentValues;
}

qint32 SamuraiBoard::makeBlockIndex (qint32 index)
{
    const int nGrids            = 5;
    const int mid               = m_order - m_overlap;
    const int low               = 2 * m_order - 2 * m_overlap;
    const int gridRow [nGrids]  = {mid, 0,   0, low, low};
    const int gridCol [nGrids]  = {mid, 0, low,   0, low};

    dbo1 "Make block index on whole board, index %d.\n", index);
    qint32 offset = index;

    // Index square blocks in the board from left to right, then top to bottom.
    int centre = mid * m_boardSize + mid;
    for (int nGrid = 0; nGrid < nGrids; nGrid++) {
        int base = gridRow [nGrid] * m_boardSize + gridCol [nGrid];
        // dbo2 "Grid %d, base %d, centre %d\n", nGrid, base, centre);
        for (int row = 0; row < m_order; row = row + m_blockSize) {
            for (int col = 0; col < m_order; col = col + m_blockSize) {
                if ((base == centre) && (m_overlap == m_blockSize)) {
                    if (((row == 0) || (row == 2 * m_blockSize)) &&
                        ((col == 0) || (col == 2 * m_blockSize))) {
                        // dbo2 "Skip overlapping block %d,%d\n", row, col);
                        continue;		// Skip the overlapping blocks.
                    }
                }
                // dbo2 "Index block %d,%d\n", row, col);
                int topLeft = base + row * m_boardSize + col;
                offset = indexSquareBlock (offset, topLeft);
            }
        }
    }

    return offset;
}

void SamuraiBoard::copyCells (int w, int h,
                              const BoardContents & from, int base1, int wRow1,
                                    BoardContents & to,   int base2, int wRow2)
{
    // Helper for SamuraiBoard::fillBoard.
    // Copy a rectangle of cells from one part of a board to another.
    //     w      Width of area to be copied,
    //     h      Height of area to be copied,
    //     from   The board from which the rectangle of cells comes,
    //     base1  The offset within the sending board's vector,
    //     wRow1  The width of the sending board,
    //     to     The board to which the rectangle of cells is going,
    //     base2  The offset within the receiving board's vector,
    //     wRow2  The width of the receiving board.

    int index1 = base1;
    int index2 = base2;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            to [index2] = from.at (index1);
            index1++;
            index2++;
        }
        index1 = index1 - w + wRow1;
        index2 = index2 - w + wRow2;
    }
}

TinySamuraiBoard::TinySamuraiBoard (QObject * parent,
                                    SudokuType sudokuType, int blockSize)
    :
    SamuraiBoard (parent, sudokuType, blockSize)
{
    m_overlap   = 1;
}	

#include "samuraiboard.moc"
