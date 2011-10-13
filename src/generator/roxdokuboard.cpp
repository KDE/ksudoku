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

#include "roxdokuboard.h"

#include <stdio.h>

RoxdokuBoard::RoxdokuBoard (QObject * parent,
                            SudokuType sudokuType, int blockSize)
    :
    PlainSudokuBoard (parent, sudokuType, blockSize)
{
    dbgLevel = 1;

    // A Roxdoku puzzle should end up with 3 * blockSize groups: a set of groups
    // for each dimension.  In the 2D layout that corresponds to blockSize
    // columns, blockSize square groups and blockSize pseudo-rows made up from
    // corresponding rows of the square groups.  On the plain Sudoku board,
    // Roxdoku uses only the leftmost blockSize columns.  The rest are marked
    // as unused and do not go into any group-indexes.

    m_nGroups = 3 * blockSize - 3 * blockSize * blockSize;

    // Set where to fill the first block in PlainSudokuBoard::fillBoard().
    // Central block at left for blockSize 3 or 5: below centre for size 4.
    m_fillStartRow = blockSize * (blockSize / 2);
    m_fillStartCol = 0;
}

void RoxdokuBoard::clear (BoardContents & boardValues)
{
    dbo1 "Clear Roxdoku grid and mark unused.\n");

    // Create an empty board of the required size.
    boardValues.fill (0, m_boardArea);

    // Roxdoku uses m_blockSize blocks in the first m_blockSize columns.
    markUnusable (boardValues, 0, m_order, m_blockSize, m_order);
}

qint32 RoxdokuBoard::makeRowColIndex (int i, int j, qint32 index)
{
    dbo1 "Make row/col index on grid at row %d col %d, index %d.\n",
            i, j, index);
    qint32 offset  = index;
    offset         = 0;
    int    topLeft = i * m_boardSize + j;
    int    cell    = topLeft;

    // Make an index of the columns in this grid.
    cell = topLeft++;
    for (int col = 0; col < m_blockSize; col++) {
        for (int row = 0; row < m_order; row++) {
            m_groupList [offset] = cell;
            dbo2 "Index col %d, row %d, value %d at offset %d.\n",
                    col, row, cell, offset);
            cell = cell + m_boardSize;
            offset++;
        }
        cell = topLeft++;
    }

    return offset;
}

qint32 RoxdokuBoard::makeBlockIndex (qint32 index)
{
    dbo1 "Make Roxdoku block index on whole board, index %d.\n", index);
    qint32 offset = index;

    // Index the blocks in the three-block board from top to bottom.
    for (int row = 0; row < m_boardSize; row = row + m_blockSize) {
        int topLeft = row * m_boardSize;
        offset = indexSquareBlock (offset, topLeft);
    }

    // Now use corresponding rows of each block to emulate the third dimension.
    int cell = 0;
    for (int k = 0; k < m_blockSize; k++) {
        for (int i = 0; i < m_boardSize; i = i + m_blockSize) {
            cell = (i + k) * m_boardSize;
            for (int j = 0; j < m_blockSize; j++) {
                m_groupList [offset] = cell;
                dbo2 "Index Roxdoku 3D block k = %d [%d,%d], "
                        "value %d at offset %d.\n",
                        k, i, j, cell, offset);
                cell++;
                offset++;
            }
        }
    }

    return offset;
}

#include "roxdokuboard.moc"
