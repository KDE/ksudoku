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

#ifndef PLAINSUDOKUBOARD_H
#define PLAINSUDOKUBOARD_H

#include "sudokuboard.h"

/**
 * @class PlainSudokuBoard  plainsudokuboard.h
 * @short Data-structures and methods for handling plain Sudoku puzzles.
 *
 * A plain or classic Sudoku puzzle has a single square grid containing smaller
 * blocks of n x n cells.  The overall grid also has n x n rows and n x n
 * columns and the rows, columns and blocks must be filled with numbers in
 * the range 1 to n x n.  In the classic form, n = 3 and numbers 1 to 9 must
 * be used in the solution.  The grid has 9 rows, 9 columns and 9 blocks.  The
 * PlainSudokuBoard class can also handle 2 x 2, 4 x 4 and 5 x 5.
 */
class PlainSudokuBoard : public SudokuBoard
{
    Q_OBJECT
public:
    /**
     * Constructs a new PlainSudokuBoard object with a required type and size.
     * This will become a puzzle of the classic Sudoku type (here called a plain
     * Sudoku), but possibly with block size other than 3x3.
     *
     * @param parent        A pointer to the object that owns this object and
     *                      will delete it automatically.
     * @param sudokuType    The type of Sudoku board required, which should
     *                      always be SudokuType::Plain.
     * @param blockSize     The size of blocks required (value 2 to 5).  The
     *                      board will have blocks, rows and columns with
     *                      blockSize squared cells and the numbers to be
     *                      filled in will range from 1 to blockSize squared
     *                      (e.g. a classic Sudoku has blockSize = 3).
     * @see globals.h
     */
    PlainSudokuBoard (QObject * parent, SudokuType sudokuType, int blockSize);

    /**
     * Sets up an empty board with the required number of cells and makes
     * indices to the required groups of cells: rows, columns and blocks.
     *
     * If blockSize = 3, the board will contain 9x9 = 81 cells and there will
     * be 9 rows, 9 columns and 9 blocks of 3x3 cells, to be filled in with
     * values 1 to 9 (i.e. a classic Sudoku).  Thus there will be 27 groups of
     * 9 cells each.  See the  SudokuBoard class description for an overview.
     */
    virtual void            setUpBoard();

    /**
     * Clear a board-vector of the required type and size. All the cells in this
     * type of Sudoku are set to zero (i.e. empty).  There are no unused cells.
     */
    virtual void            clear (BoardContents & boardValues);

    /**
     * Fill the board with randomly chosen valid values, thus generating a
     * solution from which a puzzle of the plain Sudoku type can be created.
     *
     * @return              The filled board-vector.
     */
    virtual BoardContents & fillBoard();

protected:
    /**
     * Make a list of cells in rows and columns of the board.  In a plain
     * Sudoku, the rows and columns indexed take up the entire board of
     * blockSize squared rows and columns.
     *
     * @param i             The row-number of the top left cell (i.e. 0).
     * @param j             The column-number of the top left cell (i.e. 0).
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     */
    virtual qint32          makeRowColIndex (int i, int j, qint32 index);

    /**
     * Make a list of cells in blocks of the board.  In a plain Sudoku a block
     * is a small square of cells of size blockSize * blockSize (e.g. 3x3).
     *
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     */
    virtual qint32          makeBlockIndex (qint32 index);
};

/**
 * @class XSudokuBoard  plainsudokuboard.h
 * @short Data-structures and methods for handling XSudoku puzzles.
 *
 * An XSudoku puzzle is exactly like a plain Sudoku puzzle, except that the two
 * diagonals of the board also form groups that must be filled with the values
 * 1 to blockSize * blockSize.
 */
class XSudokuBoard : public PlainSudokuBoard
{
    Q_OBJECT
public:
    /**
     * Constructs a new XSudokuBoard object with a required type and size.
     *
     * @param parent        A pointer to the object that owns this object and
     *                      will delete it automatically.
     * @param sudokuType    The type of Sudoku board required, which should
     *                      always be SudokuType::XSudoku.
     * @param blockSize     The size of blocks required (value 3 to 5).  The
     *                      board will have blocks, rows and columns with
     *                      blockSize squared cells and the numbers to be
     *                      filled in will range from 1 to blockSize squared.
     * @see globals.h
     */
    XSudokuBoard (QObject * parent, SudokuType sudokuType, int blockSize);

protected:
    /**
     * Make a list of cells in blocks of the board.  This exactly the same as
     * in a plain Sudoku, but with two blocks added, to represent the fact
     * that the diagonals of the overall grid must also contain the numbers
     * 1 to blockSize * blockSize (e.g. 1 to 9).
     *
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     */
    virtual qint32          makeBlockIndex (qint32 index);
};

/**
 * @class JigsawBoard  plainsudokuboard.h
 * @short Data-structures and methods for handling Jigsaw Sudoku puzzles.
 *
 * A Jigsaw Sudoku puzzle is exactly like a plain Sudoku puzzle, except that
 * only the central block is square.  The surrounding blocks have tabs and holes
 * and interlock like jigsaw puzzle pieces.  The block size is always 3.
 */
class JigsawBoard : public PlainSudokuBoard
{
    Q_OBJECT
public:
    /**
     * Constructs a new JigsawBoard object with required type and size.
     *
     * @param parent        A pointer to the object that owns this object and
     *                      will delete it automatically.
     * @param sudokuType    The type of Sudoku board required, which should
     *                      always be SudokuType::Jigsaw.
     * @param blockSize     The size of blocks required (must be 3).  The board
     *                      will have blocks, rows and columns with 3x3 = 9
     *                      cells and the numbers to be filled in will range
     *                      from 1 to 9.
     * @see globals.h
     */
    JigsawBoard (QObject * parent, SudokuType sudokuType, int blockSize);

protected:
    /**
     * Make a list of cells in blocks of the board.  This differs completely
     * from the plain Sudoku's makeBlockIndex() because of the irregular
     * arrangement of the blocks.
     *
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     */
    virtual qint32          makeBlockIndex (qint32 index);
};

/**
 * @class AztecBoard  plainsudokuboard.h
 * @short Data-structures and methods for handling Aztec Sudoku puzzles.
 *
 * An Aztec Sudoku puzzle is exactly like a plain Sudoku puzzle, except that
 * only the central block is square.  The surrounding blocks interlock like
 * jigsaw puzzle pieces, resembling an Aztec pyramid.  The block size is 3.
 */
class AztecBoard : public PlainSudokuBoard
{
    Q_OBJECT
public:
    /**
     * Constructs a new AztecBoard object with required type and size.
     *
     * @param parent        A pointer to the object that owns this object and
     *                      will delete it automatically.
     * @param sudokuType    The type of Sudoku board required, which should
     *                      always be SudokuType::Aztec.
     * @param blockSize     The size of blocks required (must be 3).  The board
     *                      will have blocks, rows and columns with 3x3 = 9
     *                      cells and the numbers to be filled in will range
     *                      from 1 to 9.
     * @see globals.h
     */
    AztecBoard (QObject * parent, SudokuType sudokuType, int blockSize);

protected:
    /**
     * Make a list of cells in blocks of the board.  This differs completely
     * from the plain Sudoku's makeBlockIndex() because of the irregular
     * arrangement of the blocks.
     *
     * @param index         The current position in the list of groups.
     * @return              The updated position in the list of groups.
     */
    virtual qint32          makeBlockIndex (qint32 index);
};

#endif // PLAINSUDOKUBOARD_H
