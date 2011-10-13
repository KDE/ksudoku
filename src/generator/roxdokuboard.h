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

#ifndef ROXDOKUBOARD_H
#define ROXDOKUBOARD_H

#include "plainsudokuboard.h"

/**
 * @class RoxdokuBoard  roxdokuboard.h
 * @short Data-structures and methods for handling Roxdoku 3-D Sudoku puzzles.
 *
 * A Roxdoku puzzle is a cube containing blockSize cubed cells that must satisfy
 * Sudoku-like rules (e.g. if blockSize is 3, the cube contains 3x3x3 = 27 cells
 * in the form of 27 small cubes, each empty or containing a number).  The cube
 * can be divided into blockSize planes of blockSize squared cells each, slicing
 * at right angles to any of the three spatial directions.  Each of these planes
 * must contain each of the numbers from 1 to blockSize squared once and only
 * once.  If blockSize is 3, there are 3 * blockSize = 9 such planes, each of
 * 3x3 = 9 cells and each to be filled in with the numbers 1 to 9.
 *
 * RoxdokuBoard represents a 3-D Sudoku puzzle as the left-hand blockSize
 * columns of a PlainSudoku board.  The remaining columns are marked as unused.
 * For example, in a 5x5x5 Roxdoku, 5 columns are used and 20 columns are marked
 * as unused.  In the 5 columns there are 5 blocks of 5x5 cells.  With this
 * arrangement, the square blocks represent exactly the planes facing the viewer
 * and the columns represent the planes that face left and right, but taken
 * apart.  The third dimension (the planes seen from above and below) are
 * represented by forming groups from corresponding rows (1st, 2nd, etc.) of
 * the square blocks.  For example, in a 3x3x3 cube, the top plane is made up
 * from the first rows of each of the three blocks of 3x3 cells.  The solver
 * can work with the resulting groups of cells and indices, without knowing
 * anything about the third dimension.
 */
class RoxdokuBoard : public PlainSudokuBoard
{
    Q_OBJECT
public:
    /**
     * Constructs a new RoxdokuBoard object with a required type and size.
     * This is like a PlainSudokuBoard object and uses all the same methods,
     * except for clear(), makeRowColIndex() and makeBlockIndex().
     *
     * @param parent        A pointer to the object that owns this object and
     *                      will delete it automatically.
     * @param sudokuType    The type of Sudoku board required, which should
     *                      always be SudokuType::Roxdoku.
     * @param blockSize     The size of cube required (must be 3 to 5).
     *
     * @see globals.h
     */
    RoxdokuBoard (QObject * parent, SudokuType sudokuType, int blockSize);

protected:
    /**
     * Clear a board-vector of the required type and size, which is the same as
     * for a PlainSudokuBoard.  Cells on the left-hand side of a RoxdokuBoard
     * are set to zero (empty).  The remaining cells are unused and set to -1.
     */
    virtual void            clear (BoardContents & boardValues);

    /**
     * Make a list of cells in the columns of the board.  There are no
     * full-width rows.
     *
     * @param i             The row-number of the top left cell (i.e. 0).
     * @param j             The column-number of the top left cell (i.e. 0).
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     */
    virtual qint32          makeRowColIndex (int i, int j, qint32 index);

    /**
     * Make a list of cells in blocks of the board.  This differs completely
     * from the plain Sudoku's makeBlockIndex() because there are fewer square
     * blocks, many unused blocks and some groups that are put together from
     * rows taken from each square block, to represent the third dimension.
     *
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     */
    virtual qint32          makeBlockIndex  (qint32 index);
};

#endif // ROXDOKUBOARD_H
