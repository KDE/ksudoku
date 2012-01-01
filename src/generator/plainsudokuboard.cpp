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

#include "plainsudokuboard.h"

#include <stdio.h>

PlainSudokuBoard::PlainSudokuBoard (QObject * parent,
                                    SudokuType sudokuType, int blockSize)
    :
    SudokuBoard (parent, sudokuType, blockSize)
{
    dbgLevel = 1;

    // Set where to fill the first block in PlainSudokuBoard::fillBoard().
    // Central block for blockSize 3 or 5: below and right of centre for 2 or 4.
    m_fillStartRow = blockSize * (blockSize / 2);
    m_fillStartCol = m_fillStartRow;
}

void PlainSudokuBoard::setUpBoard()
{
    m_boardSize = m_order;
    m_boardArea = m_gridArea;		// Area of the whole board.

    // Create an empty board of the required size.
    clear (m_currentValues);

    // Make a list of groups that must satisfy Sudoku rules.  These are rows,
    // columns and blocks with m_order values in each.  Note that the
    // XSudokuBoard derived class starts m_nGroups at 2, rather than 0, to
    // allow for the two diagonals and the RoxdokuBoard derived class (3D
    // puzzle) calculates its own particular value of m_nGroups.

    m_nGroups   = m_nGroups + 3 * m_order;
    m_groupSize = m_order;
    m_groupList.fill (0, m_nGroups * m_groupSize);

    qint32 index = 0;
    index = makeRowColIndex (0, 0, index);
    index = makeBlockIndex  (index);
    dbo1 "The index reached %d.\n", index);

    // Make the solver faster (see SudokuBoard::updateValueRequirements()).
    indexCellsToGroups();

    dbo1 "Block %dx%d = %d, grid %dx%d = %d, board %dx%d = %d\n",
        m_blockSize, m_blockSize, m_blockSize * m_blockSize,
        m_order, m_order, m_gridArea,
        m_boardSize, m_boardSize, m_boardArea);
    dbo1 "There are %d groups of %d cells each.\n", m_nGroups, m_groupSize);
    dbo2 "Groups are:\n");
    for (int g = 0; g < m_nGroups; g++) {
        for (int n = 0; n < m_groupSize; n++) {
            dbo2 " %3d", m_groupList.at (g * m_groupSize + n));
        }
        dbo2 "\n");
    }
}

void PlainSudokuBoard::clear (BoardContents & boardValues)
{
    // Create an empty board of the required size.
    boardValues.fill (0, m_boardArea);
}

BoardContents & PlainSudokuBoard::fillBoard()
{
    // Solve the empty board, thus filling it with values at random.  These
    // values can be the starting point for generating a puzzle and also the 
    // final solution of that puzzle.

    QVector<int> sequence (m_order);
    randomSequence (sequence);
    clear (m_currentValues);

    // Fill a central block with values 1 to m_order in random sequence.  This
    // reduces the solver time considerably, especially if blockSize is 4 or 5.
    int row, col;
    for (int n = 0; n < m_order; n++) {
        row = m_fillStartRow + (n / m_blockSize);
        col = m_fillStartCol + (n % m_blockSize);
        m_currentValues [row * m_order + col] = sequence.at (n) + 1;
    }

    solveBoard (m_currentValues);
    dbo "BOARD FILLED\n");
    return m_currentValues;
}

qint32 PlainSudokuBoard::makeRowColIndex (int i, int j, qint32 index)
{
    dbo1 "Make row/col index on grid at row %d col %d, index %d.\n",
            i, j, index);
    qint32 offset  = index;
    int    topLeft = i * m_boardSize + j;
    int    cell    = topLeft;

    // Make an index of the rows in this grid.
    for (int row = 0; row < m_order; row++) {
        for (int col = 0; col < m_order; col++) {
            m_groupList [offset] = cell;
            // dbo2 "Index row %d, col %d, value %d at offset %d.\n",
                    // row, col, cell, offset);
            cell++;
            offset++;
        }
        cell = cell + m_boardSize - m_order;
    }

    // Make an index of the columns in this grid.
    cell = topLeft++;
    for (int col = 0; col < m_order; col++) {
        for (int row = 0; row < m_order; row++) {
            m_groupList [offset] = cell;
            // dbo2 "Index col %d, row %d, value %d at offset %d.\n",
                    // col, row, cell, offset);
            cell = cell + m_boardSize;
            offset++;
        }
        cell = topLeft++;
    }

    return offset;
}

qint32 PlainSudokuBoard::makeBlockIndex (qint32 index)
{
    dbo1 "Make block index on whole board, index %d.\n", index);
    qint32 offset = index;

    // Index the blocks in the board from left to right, then top to bottom.
    // Note: The blocks must be square, but an index like this could be
    // constructed, in principle, for irregular blocks, as in Jigsaw Sudoku.

    for (int row = 0; row < m_boardSize; row = row + m_blockSize) {
        for (int col = 0; col < m_boardSize; col = col + m_blockSize) {
            int topLeft = row * m_boardSize + col;
            offset = indexSquareBlock (offset, topLeft);
        }
    }

    return offset;
}

XSudokuBoard::XSudokuBoard (QObject * parent,
                            SudokuType sudokuType, int blockSize)
    :
    PlainSudokuBoard (parent, sudokuType, blockSize)
{
    m_nGroups = 2;		// Allow two more groups for the diagonals.
}

qint32 XSudokuBoard::makeBlockIndex (qint32 index)
{
    // Make a normal Sudoku block index.
    qint32 offset = PlainSudokuBoard::makeBlockIndex (index);

    // Add cells on the diagonals to the group-indexes.
    dbo1 "XSudoku: Index the cells on the diagonals, index %d.\n", index);
    int cell_1 = 0;
    int cell_2 = m_order - 1;

    offset = offset + m_order;	// 2 * m_order cells get added to the list.
    for (int n = 0; n < m_order; n++) {
        m_groupList [offset - m_order] = cell_1;
        m_groupList [offset]           = cell_2;
        cell_1 = cell_1 + m_order + 1;
        cell_2 = cell_2 + m_order - 1;
        offset++;
    }

    return offset;
}

JigsawBoard::JigsawBoard (QObject * parent,
                          SudokuType sudokuType, int blockSize)
    :
    PlainSudokuBoard (parent, sudokuType, blockSize)
{
}

qint32 JigsawBoard::makeBlockIndex (qint32 index)
{
    dbo1 "Make Jigsaw block index on whole board, index %d\n", index);

    // Nine blocks.  The central block is square, the outside eight have
    // interlocking tabs and sockets (holes), like a jigsaw.  The cell numbers
    // run from left-to-right and then top-to-bottom.

    const int blocks[] = { 0,  1,  2,  9, 10, 11, 18, 12, 20, // Tab 12, hole 19
                           3,  4,  5, 15, 13, 14, 21, 22, 23, // Tab 15, hole 12
                           6,  7,  8, 34, 16, 17, 24, 25, 26, // Tab 34, hole 15
                          27, 28, 29, 36, 37, 38, 45, 19, 47, // Tab 19, hole 46
                          30, 31, 32, 39, 40, 41, 48, 49, 50, // Central square
                          33, 61, 35, 42, 43, 44, 51, 52, 53, // Tab 61, hole 34
                          54, 55, 56, 63, 64, 46, 72, 73, 74, // Tab 46, hole 65
                          57, 58, 59, 66, 67, 65, 75, 76, 77, // Tab 65, hole 68
                          60, 68, 62, 69, 70, 71, 78, 79, 80  // Tab 68, hole 61
                         };
    qint32 nCells = m_order * m_order;

    // Copy the blocks into the block index.
    for (int n = 0; n < nCells; n++) {
        m_groupList [index + n] = blocks [n];
    }

    return (index + nCells);
}

AztecBoard::AztecBoard (QObject * parent,
                          SudokuType sudokuType, int blockSize)
    :
    PlainSudokuBoard (parent, sudokuType, blockSize)
{
}

qint32 AztecBoard::makeBlockIndex (qint32 index)
{
    dbo1 "Make Aztec block index on whole board, index %d\n", index);

    // Nine blocks.  The central block is square, the outside eight have
    // interlocking tabs and sockets (holes), like a jigsaw.  The cell numbers
    // run from left-to-right and then top-to-bottom.

    const int blocks[] = { 0,  1,  9, 10, 11, 19, 20, 21, 29, // Top left
                           2,  3,  4,  5,  6, 12, 13, 14, 22, // Top middle
                           7,  8, 15, 16, 17, 23, 24, 25, 33, // Top right
                          18, 27, 28, 36, 37, 38, 45, 46, 54, // Middle left
                          30, 31, 32, 39, 40, 41, 48, 49, 50, // Central square
                          26, 34, 35, 42, 43, 44, 52, 53, 62, // Middle right
                          47, 55, 56, 57, 63, 64, 65, 72, 73, // Bottom left
                          58, 66, 67, 68, 74, 75, 76, 77, 78, // Bottom middle
                          51, 59, 60, 61, 69, 70, 71, 79, 80  // Bottom right
                         };
    qint32 nCells = m_order * m_order;

    // Copy the blocks into the block index.
    for (int n = 0; n < nCells; n++) {
        m_groupList [index + n] = blocks [n];
    }

    return (index + nCells);
}

#include "plainsudokuboard.moc"
