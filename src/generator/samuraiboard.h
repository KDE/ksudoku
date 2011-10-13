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

#ifndef SAMURAIBOARD_H
#define SAMURAIBOARD_H

#include "plainsudokuboard.h"

/**
 * @class SamuraiBoard  samuraiboard.h
 * @short Data-structures and methods for handling Samurai Sudoku puzzles.
 *
 * A Samurai Sudoku puzzle contains five overlapping plain Sudoku puzzles,
 * arranged in an X-shape.  Each puzzle contains the usual rows, columns and
 * blocks for the value of blockSize supplied, which must be 3 to 5.  The
 * central grid has four blocks at its corners that overlap corner blocks of
 * the four outer grids.  If blockSize is 3, the overlap is also 3.  Then there
 * will be 5x9 rows, 5x9 columns and 5x9 - 4 blocks to be solved, making a total
 * of 45 + 45 + 41 = 131 groups.  The overall board size will be 21x21 = 441 and
 * there will be unused cells in the middle of each side. 
 */
class SamuraiBoard : public PlainSudokuBoard
{
    Q_OBJECT
public:
    /**
     * Constructs a new SamuraiBoard object with a required type and size.
     *
     * @param parent        A pointer to the object that owns this object and
     *                      will delete it automatically.
     * @param sudokuType    The type of Sudoku board required, which should
     *                      always be SudokuType::Samurai.
     * @param blockSize     The size of blocks required (value 3 to 5).  The
     *                      board will have blocks, rows and columns with
     *                      blockSize squared cells and the numbers to be
     *                      filled in will range from 1 to blockSize squared.
     *
     * @see globals.h
     */
    SamuraiBoard (QObject * parent, SudokuType sudokuType, int blockSize);

    /**
     * Sets up an empty board with the required number of cells and makes
     * indices to the required groups of cells: rows, columns and blocks.
     *
     * If blockSize = 3, the board will contain 21x21 = 441 cells and there will
     * be 45 rows, 45 columns and 41 blocks of 3x3 cells, to be filled in with
     * values 1 to 9.  4x2x9 = 72 cells will be unused and will not appear in
     * any indices of groups.  In all, there will be 131 groups of 9 cells each.
     */
    virtual void            setUpBoard();

    /**
     * Clear a board-vector of the required type and size. Most cells in this
     * type of Sudoku are set to zero (empty), but unused cells are set to -1.
     */
    virtual void            clear (BoardContents & boardValues);

    /**
     * Fill the five overlapping Sudoku boards with randomly chosen valid
     * values, thus generating a solution from which a puzzle of the Samurai
     * or TinySamurai type can be created.
     */
    virtual BoardContents & fillBoard();

protected:
    /**
     * Make a list of cells in blocks of the board.  This differs completely
     * from the plain Sudoku's makeBlockIndex() because of the larger number
     * of blocks and the presence of overlapped and unused blocks.
     *
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     *
     * @note SamuraiBoard uses PlainSudokuBoard::makeRowColIndex(), by
     * inheritance, for indexing rows and columns.
     */
    virtual qint32          makeBlockIndex  (qint32 index);

private:
    // Copy a rectangle of cells from one part of a board to another.
    void copyCells (int w, int h,
                    const BoardContents & from, int base1, int wRow1,
                          BoardContents & to,   int base2, int wRow2);

};

/**
 * @class TinySamuraiBoard  samuraiboard.h
 * @short Data-structures and methods for handling Tiny Samurai Sudoku puzzles.
 *
 * A Tiny Samurai Sudoku puzzle contains five overlapping plain Sudoku puzzles,
 * with grid-area 4x4, arranged in an X-shape.  The blockSize must be 2.  Each
 * puzzle contains the 5x4 rows, 5x4 columns and 5x4 blocks of 2x2 cells.  There
 * are no overlapping blocks, just an overlapping cell at each corner of the
 * central 4x4 grid.  A Tiny Samurai puzzle is much easier than a full Samurai.
 */
class TinySamuraiBoard : public SamuraiBoard
{
    Q_OBJECT
public:
    /**
     * Constructs a new TinySamuraiBoard object with a required type and size.
     * This is exactly like a SamuraiBoard object and uses all the same methods,
     * except that the overlap is set to 1 cell and the blockSize is always 2.
     *
     * @param parent        A pointer to the object that owns this object and
     *                      will delete it automatically.
     * @param sudokuType    The type of Sudoku board required, which should
     *                      always be SudokuType::TinySamurai.
     * @param blockSize     The size of blocks required (must be 2).  The board
     *                      will have blocks, rows and columns with 2x2 = 4
     *                      cells and the numbers to be filled in will range
     *                      from 1 to 4.
     *
     * @see globals.h
     */
    TinySamuraiBoard (QObject * parent, SudokuType sudokuType, int blockSize);
};

#endif // SAMURAIBOARD_H
