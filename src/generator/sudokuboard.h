/****************************************************************************
 *    Copyright 2011  Ian Wadham <iandw.au@gmail.com>                       *
 *    Copyright 2006  David Bau <david bau @ gmail com> Original algorithms *
 *    Copyright 2015  Ian Wadham <iandw.au@gmail.com>                       *
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

#ifndef SUDOKUBOARD_H
#define SUDOKUBOARD_H

#include "globals.h"
#include "skgraph.h"

#include <QList>
#include <QObject>
#include <QStack>

using Pair = qint32;		// Two small integers packed into one.

using Move = Pair;
using MoveList = QList<Move>;

using Guess = Move;
using GuessesList = MoveList;

enum                 GuessingMode {Random, NotRandom};

class SKGraph;
class State;

// TODO - SudokuBoard, MathdokuGenerator, CageGenerator and DLXSolver could be
//        factored better. At the moment, MathdokuGenerator needs SudokuBoard's
//        fillBoard() method to create a square that satisfies Sudoku rules for
//        Killer Sudoku or Mathdoku puzzles. But fillBoard() depends on large
//        parts of SudokuBoard's solver logic... so we have two solver objects
//        co-existing for now, but this happens only for a second or so.

/**
 * @class SudokuBoard  sudokuboard.h
 * @short Generalized data-structures and methods for handling Sudoku puzzles.
 *
 * SudokuBoard is an abstract class for handling several types of Sudoku puzzle,
 * including the classic 9x9 Sudoku, other sizes of the classic Sudoku, the
 * XSudoku and Jigsaw variants, Samurai Sudoku (with five overlapping grids)
 * and the three-dimensional Roxdoku.
 *
 * The class is an adaptation of algorithms in a Python program, Copyright (c)
 * David Bau 2006, which appears at http://davidbau.com/downloads/sudoku.py and
 * is discussed at http://davidbau.com/archives/2006/09/04/sudoku_generator.html
 * 
 * A puzzle, its solution and the intermediate steps in solution are represented
 * as vectors of integer cells (type BoardContents), in which a cell can contain
 * zero if it is yet to be solved, -1 if it is not used (e.g. the gaps between
 * the five overlapping grids of a Samurai Sudoku) or an integer greater than
 * zero if it is a given (or clue) or has been (tentatively) solved.
 *
 * The central method of the class is the solver (solve()). It is used when
 * solving an existing puzzle, generating a new puzzle, checking the validity of
 * a puzzle keyed in or loaded from a file, verifying that a puzzle is solvable,
 * checking that it has only one solution and collecting statistics related to
 * the difficulty of solving the puzzle.
 *
 * Puzzle generation begins by using the solver to fill a mainly empty board and
 * thus create the solution.  The next step is to insert values from the
 * solution into another empty board until there are enough to solve the puzzle
 * without any guessing (i.e. by logic alone).  If the difficulty of the puzzle
 * is now as required, puzzle generation finishes.  It it is too hard, a few
 * more values are inserted and then puzzle generation finishes.
 *
 * If the puzzle is not yet hard enough, some of the values are removed at
 * random until the puzzle becomes insoluble if any more cells are removed or
 * the puzzle has more than one solution or the required level of difficulty
 * is reached.  If the puzzle is still not hard enough after all random removals
 * have been tried, the whole puzzle-generation process is repeated until the
 * required difficulty is reached or a limit is exceeded.
 *
 * The principal methods used in puzzle-generation are generatePuzzle(),
 * insertValues(), removeValues() and checkPuzzle().  The checkPuzzle() method
 * is also used to check the validity of a puzzle entered manually or loaded
 * from a file.  The virtual methods clear() and fillBoard() clear a board or
 * fill it with randomly chosen values (the solution).
 *
 * The main input to the puzzle generator/solver is a pointer to an object of
 * type SKGraph.  That object contains the shape, dimensions and rules for
 * grouping the cells of the particular type of Sudoku being played, including
 * Classic Sudoku in several sizes and variants, Samurai Sudoku with five
 * overlapping grids and the three-dimensional Roxdoku in several sizes.
 *
 * Each group (row, column, block or plane) contains N cells in which the
 * numbers 1 to N must appear exactly once.  N can be 4, 9, 16 or 25, but not
 * all types of puzzle support all four sizes.
 *
 * As examples, a classic Sudoku puzzle has 27 groups: 9 rows, 9 columns and
 * 9 blocks of 3x3 cells.  Each group must contain the numbers 1 to 9 once and
 * only once.  An XSudoku puzzle has two extra groups of 9 cells each on the
 * board's diagonals.  A Samurai puzzle has five overlapping 9x9 grids, with 45
 * columns, 45 rows and 41 blocks of 3x3 cells, making 131 groups in all.  A
 * classic Sudoku puzzle of size 16 has 16 rows, 16 columns and 16 blocks of
 * 4x4 cells, making 48 groups, where each group must contain the values 1 to
 * 16 once and only once.  A 3x3x3 Roxdoku puzzle is a cube with 9 groups of
 * 3x3 cells.  These form 3 planes perpendicular to each of the X, Y and Z axes.
 *
 * All these configurations are represented by a table of groups (or cliques) in
 * the SKGraph object, which maps cell numbers into groups.  The SudokuBoard
 * class itself is unaware of the type of puzzle it is generating or solving.
 */
class SudokuBoard : public QObject
{
    Q_OBJECT
public:
    /**
     * Construct a new SudokuBoard object with a required type and size.
     *
     * @param graph         The layout, type and size of the board, including
     *                      the grouping of cells into rows, columns and blocks,
     *                      as required by the type of puzzle being played.
     */
    explicit SudokuBoard (SKGraph * graph);

    /**
     * Generate a puzzle and its solution (see details in the class-header doc).
     *
     * @param puzzle        The generated puzzle.
     * @param solution      The generated solution.
     * @param difficulty    The required level of difficulty (as defined in file
     *                      globals.h).
     * @param symmetry      The required symmetry of layout of the clues.
     *
     * @return              Normally true, but false if the user wishes to go
     *                      back to the Welcome screen (e.g. to change reqs.)
     *                      after too many attempts to generate a puzzle.
     */
    bool                    generateMathdokuKillerSudokuTypes (BoardContents & puzzle,
                                            BoardContents & solution,
                                            Difficulty      difficulty,
                                            Symmetry        symmetry);
    bool                    generateSudokuRoxdokuTypes (BoardContents & puzzle,
                                                       BoardContents & solution,
                                                       Difficulty    difficulty,
                                                       Symmetry      symmetry);


    /**
     * Check that a puzzle is soluble, has the desired solution and has only one
     * solution.  This method can be used to check puzzles loaded from a file or
     * entered manually, in which case the solution parameter can be omitted.
     *
     * @param puzzle        The board-contents of the puzzle to be checked.
     * @param solution      The board-contents of the desired solution if known.
     *
     * @return              The result of the check, with values as follows:
     * @retval >= 0         The difficulty of the puzzle, approximately, after
     *                      one solver run.  If there are guesses, the
     *                      difficulty can vary from one run to another,
     *                      depending on which guesses are randomly chosen.
     * @retval -1           No solution.
     * @retval -2           Wrong solution.
     * @retval -3           More than one solution.
     */
    int                     checkPuzzle (const BoardContents & puzzle,
                                         const BoardContents & solution =
                                               BoardContents());

    /**
     * Provide a list of solution moves for use as KSudoku hints.
     *
     * @param moveList     A list of KSudoku indices of solution moves (output).
     */
    void getMoveList (QList<int> & moveList);

    /**
     * Calculate the difficulty of a puzzle, based on the number of guesses
     * required to solve it, the number of iterations of the solver's deduction
     * method over the whole board and the fraction of clues or givens in the
     * starting position.  Easier levels of difficulty involve logical deduction
     * only and would usually not require guesses: harder levels might.
     *
     * When guesses are involved (i.e. branches or forks in the solution
     * process), the calculated difficulty can vary from one run to another of
     * the solver, depending on which guesses are randomly chosen, so it is
     * necessary to do several runs (e.g. 5) and calculate average values.
     *
     * @param puzzle        The board-contents of the puzzle to be assessed.
     * @param nSamples      The number of solver runs over which to take an
     *                      average.
     *
     * @return              The estimated difficulty of the puzzle (as defined
     *                      in file globals.h).
     */
    Difficulty              calculateRating (const BoardContents & puzzle,
                                             int nSamples = 5);

    /**
     * Solve a puzzle and return the solution.
     *
     * @param boardValues   The board-contents of the puzzle to be solved.
     *
     * @return              The board-contents of the solution.
     */
    BoardContents &         solveBoard (const BoardContents & boardValues,
                                              GuessingMode gMode = Random);

    /**
     * Fill the board with randomly chosen valid values, thus generating a
     * solution from which a puzzle can be created (virtual).  It is made
     * public so that it can be used to fill a Mathdoku or Killer Sudoku
     * board with numbers that satisfy Sudoku constraints.
     *
     * @return              The filled board-vector.
     */
    virtual BoardContents & fillBoard();

    /**
     * Initialize or re-initialize the random number generator.
     */
    void                    setSeed();
	QString	                getMessage() { return messageString; };
    
protected:
    BoardContents           m_currentValues;	///< The current state of the
						///< cell values during solve().

    SudokuType	            m_type;		///< The type of Sudoku puzzle
						///< (see file globals.h).
    int                     m_order;		///< The number of cells per
						///< row, column or block (4,
						///< 9, 16 or 25).
    int                     m_blockSize;	///< The number of cells on one
						///< side of a square block (2,
						///< 3, 4 or 5).
    int                     m_boardSize;	///< The number of cells on one
						///< side of the whole board.
						///< In Samurai with 9x9 grids,
						///< this is 21.
    int                     m_boardArea;	///< The number of cells in the
						///< whole board.  In Samurai
						///< with 9x9 grids this is 441.
    int                     m_overlap;		///< The degree of overlap in a
						///< Samurai board (=m_blockSize
						///< or 1 for a TinySamurai).
    int                     m_nGroups;		///< The total number of rows,
						///< columns, blocks, diagonals,
						///< etc. in the puzzle.
    int                     m_groupSize;	///< The number of cells in each
						///< group (= m_order).
    QList<int>              m_cellIndex;	///< A first-level index from a
						///< cell to the list of groups
						///< to which it belongs.
    QList<int>              m_cellGroups;	///< A second-level index from
						///< cells to individual groups
						///< to which they belong.

    /**
     * Clear a board-vector of the required type and size (virtual).
     *
     * @param boardValues   The board-contents to be cleared.
     */
    virtual void            clear (BoardContents & boardValues);

    /*
     * Fill a vector of integers with values from 1 up to the size of the
     * vector, then shuffle the integers into a random order.
     *
     * @param sequence      The vector to be filled.
     */
    void                    randomSequence (QList<int> & sequence);

private:
	QString                 messageString;

    SKGraph *               m_graph;
    int                     m_vacant;
    int                     m_unusable;
    enum                    MoveType {Single, Spot, Guess, Wrong,
                                      Deduce, Result};
    Statistics              m_stats;
    Statistics              m_accum;
    MoveList                m_moves;
    MoveList                m_moveTypes;
    QList<int>              m_KSudokuMoves;	// Move-list for KSudoku hints.

    QStack<State *>         m_states;

    QList<qint32>           m_validCellValues;
    QList<qint32>           m_requiredGroupValues;
    // These are the principal methods of the solver.  The key method is
    // deduceValues().  It finds and fills cells that have only one possible
    // value left.  If no more cells can be deduced, it returns a randomised
    // list of guesses.  Very easy to Medium puzzles are usually entirely
    // deducible, so solve() begins by trying that path.  If unsuccessful, it
    // uses tryGuesses() to explore possibilities and backtrack when required.

    BoardContents &         solve          (GuessingMode gMode);
    BoardContents &         tryGuesses     (GuessingMode gMode);
    GuessesList             deduceValues   (BoardContents & cellValues,
                                            GuessingMode gMode);
    GuessesList             solutionFailed (GuessesList & guesses);

    // These methods set up and maintain bit maps that indicate which values are
    // (still) allowed in each cell and which values are (still) required in
    // each group (row, column or block).

    void                    setUpValueRequirements
                                      (BoardContents & boardValues);
    void                    updateValueRequirements
                                      (BoardContents & boardValues, int cell);

    /**
     * Clear a board-vector and insert values into it from a solved board.  As
     * each value is inserted, it is copied into a parallel board along with
     * cells that can now be deduced logically.  The positions of values to be
     * inserted are chosen at random.  The process finishes when the parallel
     * board is filled, leaving a puzzle board that is only partly filled but
     * for which the solution can be entirely deduced without any need to guess
     * or backtrack.  However this could still be a difficult puzzle for a human
     * solver.  If the difficulty is greater than required, further values are
     * inserted until the puzzle reaches the required difficulty.
     *
     * @param solution      The solved board from which values are selected for
     *                      insertion into the puzzle board.
     * @param difficulty    The required level of difficulty (as defined in file
     *                      globals.h).
     * @param symmetry      The required symmetry of layout of the clues.
     *
     * @return              The puzzle board arrived at so far.
     */
    BoardContents           insertValues (const BoardContents & solution,
                                          const Difficulty      difficulty,
                                          const Symmetry        symmetry);

    /**
     * Remove values from a partially generated puzzle, to make it more
     * difficult.  As each value is removed, there is a check that the puzzle
     * is soluble, has the desired solution and has only one solution.  If it
     * fails this check, the value is replaced and another value is tried.  The
     * resulting puzzle could require one or more guesses, perhaps with some
     * backtracking.  The positions of values to be removed are chosen in a
     * random order.  If the required difficulty is "Unlimited", this algorithm
     * can generate "inhuman" puzzles that are extremely difficult and tedious
     * for a person to solve and can also be rather boring.
     *
     * This tendency is controlled in two ways.  Firstly, there is a minimum
     * percentage of the board that must be filled with clues and deducible
     * moves before a puzzle with guesses (i.e. branches or forks) is allowed.
     * Secondly, when the required difficulty is reached, removed values are
     * saved in a list until the required difficulty is exceeded.  Then half
     * the saved values are put back.  This "middle road" is chosen because, at
     * the transition points, the difficulty can vary between runs of the solver
     * if (random) guessing is required.
     *
     * @param solution      The board-contents of the desired solution.
     * @param puzzle        The board-contents of the partly generated puzzle.
     * @param difficulty    The required level of difficulty (as defined in file
     *                      globals.h).
     * @param symmetry      The required symmetry of layout of the clues.
     *
     * @return              The pruned puzzle board.
     */ 
    BoardContents           removeValues (const BoardContents & solution,
                                                BoardContents & puzzle,
                                          const Difficulty      difficulty,
                                          const Symmetry        symmetry);

    /**
     * Compile statistics re solution moves and calculate a difficulty rating
     * for the puzzle, based on the number of guesses required, the number of
     * iterations of the deducer over the whole board and the fraction of clues
     * provided in the starting position.
     *
     * @param s             A structure containing puzzle and solution stats.
     */
    void                    analyseMoves (Statistics & s);

    /**
     * Convert the difficulty rating of a puzzle into a difficulty level.
     *
     * @param rating        The difficulty rating.
     *
     * @return              The difficulty level, from VeryEasy to Unlimited, as
     *                      defined in file globals.h).
     */
    Difficulty              calculateDifficulty (float rating);

    /**
     * Add or clear one clue or more in a puzzle, depending on the symmetry.
     *
     * @param to            The puzzle grid to be changed.
     * @param cell          The first cell to be changed.
     * @param type          The type of symmetry.
     * @param from          The grid from which the changes are taken.
     */
    void                    changeClues (BoardContents & to,
                                         int cell, Symmetry type,
                                         const BoardContents & from);
    /**
     * For a given cell, calculate the positions of cells that satisfy the
     * current symmetry requirement.
     *
     * @param size          The size of one side of the board.
     * @param type          The required type of symmetry (if any).
     * @param cell          The position of a selected cell.
     * @param out[4]        A set of up to four symmetrically placed cells.
     *
     * @return              The number of symmetrically placed cells.
     */
    int                     getSymmetricIndices (int size, Symmetry type,
                                                 int cell, int * out);

    /**
     * Format some board-contents into text for printing, debugging output or
     * saving to a file that can be loaded.
     *
     * @param boardValues   The contents of the board to be formatted.
     */
    void                    print (const BoardContents & boardValues);

    // Methods for packing two small integers into one and unpacking them.  Used
    // for speed and efficiency in the solver and other places.
    inline Pair             setPair (int p, int v ) { return (p << 8) + v; }
    inline int              pairPos (const Pair x)  { return (x >> 8);     }
    inline int              pairVal (const Pair x)  { return (x & 255);    }
};

#endif // SUDOKUBOARD_H
