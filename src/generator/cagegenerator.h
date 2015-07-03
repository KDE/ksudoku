/****************************************************************************
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

#ifndef CAGEGENERATOR_H
#define CAGEGENERATOR_H

#include <QObject>
#include <QVector>

#include "globals.h"
#include "sudokuboard.h"

enum Direction {ALONE = 0, N = 1, E = 2, S = 4, W = 8, TAKEN = 15};

class SKGraph;
class DLXSolver;

/**
 * This class and its methods do all the work of generating a Mathdoku or
 * Killer Sudoku puzzle, starting from a solved set of cell-values in a Sudoku
 * grid. It lays down a pattern of irregular shaped cages, of different sizes,
 * which together cover the grid. Cages of size 1 have only one possible
 * solution, so they act as givens or clues. Cages of larger size are given an
 * operator (+*-/) and a target value. In the solution, the values in each cage
 * must combine together, using the operator, to equal the target value. Finally
 * the puzzle, represented by the targets, the operators and the single cells,
 * must have a unique solution. The DLX solver tests this. If there is no unique
 * solution, the puzzle must be rejected and the user of this class will need to
 * try again.
 *
 * In Killer Sudoku, the only operator is +, there are there are square boxes
 * (as well as rows and columns) that must satisfy Sudoku rules and a cage
 * cannot contain the same digit more than once.
 *
 * In Mathdoku (aka KenKen TM), all four operators can occur, a digit can occur
 * more than once in a cage and Sudoku rules apply only to rows and columns. The
 * latter means that a Mathdoku puzzle can have any size from 3x3 up to 9x9.
 * Division and subtraction operators are a special case. They can only appear
 * in cages of size 2. This is because the order in which you do divisions or
 * subtractions, in a cage of size 3 or more, can affect the result. 6 - (4 - 1)
 * = 3, but (6 - 4) - 1 = 1.
 *
 * @short A generator for Mathdoku and Killer Sudoku puzzles
 */
class CageGenerator : public QObject
{
    Q_OBJECT
public:
    CageGenerator (const BoardContents & solution);
    virtual ~CageGenerator();

    /**
     * Fill the puzzle area with Mathdoku or Killer Sudoku cages. The graph
     * parameter indicates the size and type of puzzle. The other parameters
     * affect its difficulty. The cages are stored in the graph object, where
     * they can be used by other objects (e.g. to display the cages).
     *
     * @param graph           An SKGraph object representing the size, geometric
     *                        layout and rules of the particular kind of puzzle.
     * @param solutionMoves   A pointer that returns an ordered list of cells
     *                        found by the solver when it reached a solution.
     * @param maxSize         The maximum number of cells a cage can have.
     * @param maxValue        The maximum total value a cage's cells can have.
     * @param hideOperators   Whether operators are to be hidden in a Mathdoku
     *                        puzzle. In a Killer Sudoku the operators are all +
     *                        and are always hidden.
     * @param maxCombos       The maximum number of possible solutions any cage
     *                        can have.
     *
     * @return                The number of cages generated, or 0 = too many
     *                        failures to make an acceptable cage, or -1 = no
     *                        unique solution to the puzzle using the cages
     *                        generated (the caller may need to try again).
     */
    int  makeCages (SKGraph * graph, QList<int> * solutionMoves,
                    int maxSize, int maxValue,
                    bool hideOperators, int maxCombos);

    /**
     * Using just the puzzle-graph and its cages, solve a Mathdoku or Killer
     * Sudoku puzzle and check that it has only one solution. This method can
     * be used with a manually entered puzzle or one loaded from a saved file,
     * to obtain solution values and a move-sequence for hints, as well as
     * checking that the puzzle and its data are valid.
     *
     * @param graph           An SKGraph object representing the size, geometric
     *                        layout and rules of the particular kind of puzzle.
     * @param solution        The solution returned if a unique solution exists.
     * @param solutionMoves   A pointer that returns an ordered list of cells
     *                        found by the solver when it reached a solution.
     * @param hideOperators   Whether operators are to be hidden in a Mathdoku
     *                        puzzle. In a Killer Sudoku the operators are all +
     *                        and are always hidden.
     *
     * @return                0  = there is no solution,
     *                        1  = there is a unique solution,
     *                        >1 = there is more than one solution.
     */
    int checkPuzzle (SKGraph * graph, BoardContents & solution,
                     QList<int> * solutionMoves, bool hideOperators);

private:
    SKGraph *     mGraph;		// The geometry of the puzzle.
    DLXSolver *   mDLXSolver;		// A solver for generated puzzles.
    BoardContents mSolution;

    int           mOrder;		// The height and width of the grid.
    int           mBoardArea;		// The number of cells in the grid.

    bool          mKillerSudoku;	// Killer Sudoku or Mathdoku rules?
    bool          mHiddenOperators;	// Operators in cages are displayed?

    // Working-data used in the cage-generation algorithm.
    QList<int>    mUnusedCells;		// Cells not yet assigned to cages.
    QList<int>    mNeighbourFlags;	// The assigned neighbours cells have.

    int           mSingles;		// The number of 1-cell cages (clues).
    int           mMinSingles;		// The minimum number required.
    int           mMaxSingles;		// The maximum number required.
    int           mMaxCombos;		// The maximum combos a cage can have.

    // mPossibilities is a list of possible combinations and values all the
    // cages might have. It is used when setting up the DLX matrix for the
    // solver and again when decoding the solver's result into a solution grid.
    //
    // The Index list indicates where the combos for each cage begin and end.
    // It is used to find the first combo for each cage and the beginning of
    // the next cage's combos. The difference between these two gives the number
    // of possible values that might solve the cage. Divide that by the size of
    // the cage to get the number of combos for that cage. One or more of these
    // combos are correct ones, which the solver must find. The last entry in
    // the index is equal to the total size of mPossibilities.

    QList<int> *  mPossibilities;
    QList<int> *  mPossibilitiesIndex;

    // PRIVATE METHODS.

    // Form a group of cells that makes up a cage of a chosen size (or less).
    QVector<int> makeOneCage (int seedCell, int requiredSize);

    // Choose an operator for the cage and calculate the cage's value.
    void setCageTarget (QVector<int> cage, CageOperator & cageOperator,
                        int & cageValue);

    // Check whether a generated cage is within parameter requirements.
    bool cageIsOK (const QVector<int> cage, CageOperator cageOperator,
                   int cageValue);

    // Set all possible values for the cells of a cage (used by the solver).
    void setAllPossibilities (const QVector<int> cage, int nDigits,
                              CageOperator cageOperator, int cageValue);

    // Set all possible values for one operator in a cage (used by the solver).
    void setPossibilities (const QVector<int> cage, CageOperator cageOperator,
                           int cageValue);

    // Set all possible values for a cage that has a multiply or add operator.
    void setPossibleAddsOrMultiplies (const QVector<int> cage,
                                      CageOperator cageOperator, int cageValue);

    // Check if a cage contains duplicate digits (not allowed in Killer Sudoku).
    bool hasDuplicates (int nDigits, QVector<int> digits);

    // Check if a combo of digits in a cage satisfies Sudoku rules (a Mathdoku
    // cage can contain a digit more than once, but not in the same row/column).
    bool isSelfConsistent (const QVector<int> cage, int nDigits,
                           QVector<int> digits);

    // Initialise the cage generator for a particular size and type of puzzle.
    void init (SKGraph * graph, bool hiddenOperators);
};

#endif // CAGEGENERATOR_H
