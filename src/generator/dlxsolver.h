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

#ifndef DLXSOLVER_H
#define DLXSOLVER_H

#include <QObject>
#include <QList>

#include "globals.h"

struct DLXNode			// Represents a 1 in a sparse matrix
				// containing only ones and zeroes.
{
    DLXNode * left;		// Link to next node left.
    DLXNode * right;		// Link to next node right.
    DLXNode * above;		// Link to next node above.
    DLXNode * below;		// Link to next node below.

    DLXNode * columnHeader;	// Link to top of column.
    int       value;		// In col header: count of ones in col.
				// In node: row-number of node.
};

/**
 * @class DLXSolver dlxsolver.h
 * @short Provides a solver, based on the DLX algorithm, for Sudoku variants.
 *
 * This solver can handle all variants of Sudoku puzzles supported by KSudoku,
 * including classical 9x9 Sudokus, 2-D variants, 3-D variants, Killer Sudoku
 * and MathDoku (aka KenKen TM).
 *
 * Killer and MathDoku puzzles have cages in which all the numbers must satisfy
 * an arithmetic constraint, as well as satisfying the usual Sudoku constraints
 * (using all numbers exactly once each in a column, row or group).  Killer
 * Sudokus have 9x9 cells and nine 3x3 boxes, but there are no clues and each
 * cage must add up to a prescribed total.  MathDokus can have N x N cells, for
 * N >= 3, but no boxes.  Each cage has an operator (+, -, multiply or divide)
 * which must be used to reach the required value.  In Killers and Mathdokus, a
 * cage with just one cell is effectively a clue.
 *
 * The DLX algorithm (aka Dancing Links) is due to Donald Knuth.
 */
class DLXSolver : public QObject
{
    Q_OBJECT
public:
    DLXSolver (QObject * parent);
    virtual   ~DLXSolver();

    /** 
     * Takes any of the various kinds of 2-D Sudoku or 3-D Roxdoku puzzle
     * supported by the KSudoku application program and converts it into a
     * sparse matrix of constraints and possible solution values for each
     * vacant cell. It then calls the solveDLX method to solve the puzzle,
     * using Donald Knuth's Dancing Links (DLX) algorithm. The algorithm can
     * find zero, one or any number of solutions, each of which can converted
     * back into a KSudoku grid containing a solution.
     *
     * Each column in the DLX matrix represents a constraint that must be
     * satisfied. In a Classic 9x9 Sudoku, there are 81 constraints to say that
     * each cell must be filled in exactly once. Then there are 9x9 constraints
     * to say that each of the 9 Sudoku columns must contain the numbers 1 to 9 
     * exactly once. Similarly for the 9 Sudoku rows and the 9 3x3 boxes. In
     * total, there are 81 + 9x9 + 9x9 + 9x9 = 324 constraints and so there are
     * 324 columns in the DLX matrix.
     *
     * Each row in the DLX matrix represents a position in the Sudoku grid and
     * a value (1 to 9) that might go there. If it does, it will satisfy 4 of
     * the constraints: filling in a cell and putting that value in a column, a
     * row and a 3x3 box. That possibility is represented by a 1 in that row in
     * each of the corresponding constraint columns. Thus there are 4 ones in
     * each row of the 9x9 Sudoku's DLX matrix and in total there 9x81 = 729
     * rows, representing a possible 1 to 9 in each of 81 cells.
     *
     * A solution to the 9x9 Sudoku will consist of a set of 81 rows such that
     * each column contains a single 1. That means that each constraint is
     * satisfied exactly once, as required by the rules of Sudoku. Each of the
     * successful 81 rows will still contain its original four 1's, representing
     * the constraints the corresponding Sudoku cell and value satisfies.
     *
     * Applying clues reduces the rows to be found by whatever the number of
     * clues is --- and it also reduces the size of the DLX matrix considerably.
     * For example, for a 9x9 Classic Sudoku, the size can reduce from 729x324
     * to 224x228 or even less. Furthermore, many of the remaining columns
     * contain a single 1 already, so the solution becomes quite fast.
     *
     * KSudoku can handle other sizes and shapes of Sudoku puzzle, including the
     * 3-D Roxdoku puzzles. For example, an XSudoku is like a Classic 9x9 puzzle
     * except that the two diagonals must each contain the numbers 1 to 9 once
     * and once only. In DLX, this can be represented by 18 additional columns
     * to represent the constraints on the diagonals. Also a group of 9 cells
     * might not have a simple row, column or 3x3 box shape, as in a jigsaw
     * type of Sudoku or a 3-D Roxdoku, and a Samurai Sudoku has five 9x9
     * grids overlapping inside a 21x21 array of cells, some of which must NOT
     * be used. All this is represented by lists of cells in the SKGraph object,
     * known as "cliques" or "groups". So, in the more general case, each group
     * of 9 cells will have its own 9 constraints or DLX matrix columns.
     *
     * @param graph          An SKGraph object representing the size, geometric
     *                       layout and rules of the particular kind of puzzle.
     * @param boardValues    A vector containing clue values, vacant cells and
     *                       unused values for the puzzle and its layout.
     * @param solutionLimit  A limit to the number of solutions to be delivered
     *                       where 0 = no limit, 1 gets the first solution only
     *                       and 2 is used to test if there is > 1 solution.
     *
     * @return               The number of solutions found (0 to solutionLimit).
     */
    int       solveSudoku   (SKGraph * graph, const BoardContents & boardValues,
                                                int solutionLimit = 2);

    /**
     * Takes a Mathdoku or Killer Sudoku puzzle and converts it into a sparse
     * matrix of constraints and possible solution values for each cage. The
     * constraints are that each cage must be filled in and that each column
     * and row of the puzzle solution must follow Sudoku rules (blocks too, in
     * Killer Sudoku). The possible solutions are represented by one DLX row per
     * possible set of numbers for each cage. The solveDLX() method is then used
     * to test that the puzzle has one and only one solution, which consists of
     * a subset of the original DLX rows (i.e. one set of numbers per cage). For
     * more detail, see solveSudoku().
     *
     * @param graph          An SKGraph object representing the size, geometric
     *                       layout and rules of the particular kind of puzzle,
     *                       as well as its cage layouts, values and operators.
     * @param solutionMoves  A pointer that returns an ordered list of cells
     *                       found by the solver when it reaches a solution.
     * @param possibilities  A pointer to a list of possible values for all the
     *                       cells in all the cages.
     * @param possibilitiesIndex
     *                       An index into the possibilities list, with one
     *                       index-entry per cage, plus an end-of-list index.
     * @param solutionLimit  A limit to the number of solutions to be delivered
     *                       where 0 = no limit, 1 gets the first solution only
     *                       and 2 is used to test if there is > 1 solution.
     *
     * @return               The number of solutions found (0 to solutionLimit).
     */
    int       solveMathdoku (SKGraph * graph, QList<int> * solutionMoves,
                             const QList<int> * possibilities,
                             const QList<int> * possibilitiesIndex,
                             int solutionLimit = 2);

    // void      testDLX();  // TODO - Delete this eventually.

private:
    /**
     * Takes a sparse matrix of ones and zeroes and solves the Exact Cover
     * Problem for it, using Donald Knuth's Dancing Links (DLX) algorithm.
     *
     * A solution is a subset of rows which, when combined, have a single 1 in
     * each column. If each DLX column represents a constraint or condition that
     * must be satisfied exactly once and each row represents a possible part of
     * the solution, then the whole matrix can represent a problem such as
     * Sudoku or Mathdoku and the subset of rows can represent a solution to
     * that Sudoku or Mathdoku. A particular matrix can have 0, 1 or any number
     * of Exact Cover solutions, as can the corresponding puzzle.
     *
     * See the code in file dlxsolver.cpp for a description of the algorithm.
     *
     * @param solutionLimit  A limit to the number of solutions to be delivered
     *                       where 0 = no limit, 1 gets the first solution only
     *                       and 2 is used to test if there is > 1 solution.
     *
     * @return               The number of solutions found (0 to solutionLimit).
     *                       Actual solutions are returned progressively a Qt
     *                       signal-slot mechanism.
     */
    int       solveDLX      (int solutionLimit);

    /**
     * Temporarily remove a column from the DLX matrix, along with all of the
     * rows that have nodes (1's) in this column.
     *
     * @param colDLX        A pointer to the header of the column.
     */
    void      coverColumn   (DLXNode * colDLX);

    /**
     * Re-insert a column into the DLX matrix, along with all of the rows that
     * have nodes (1's) in this column.
     *
     * @param colDLX        A pointer to the header of the column.
     */
    void      uncoverColumn (DLXNode * colDLX);

    void recordSolution (const int solutionNum, QList<DLXNode *> & solution);

    // Empty the DLX matrix, but do not deallocate the nodes.
    void      clear();

    // Add a node (i.e. a 1) to the sparse DLX matrix.
    void      addNode       (int rowNum, int colNum);

    // Get a node-structure, allocating or re-using as needed.
    DLXNode * allocNode();

    // Initialise a node to point to itself and contain value 0.
    void      initNode      (DLXNode * node);

    // Circularly link a node to the end of a DLX matrix row.
    void      addAtRight    (DLXNode * node, DLXNode * start);

    // Circularly link a node to the end of a DLX matrix column.
    void      addBelow      (DLXNode * node, DLXNode * start);

    // Deallocate all nodes.
    void      deleteAll();

    DLXNode *        mCorner;
    QList<DLXNode *> mColumns;
    QList<DLXNode *> mRows;
    QList<DLXNode *> mNodes;
    int              mEndColNum;
    int              mEndRowNum;
    int              mEndNodeNum;

    BoardContents    mBoardValues;	// Holds Sudoku problem and solution.
    QList<int> *     mSolutionMoves;	// Sequence of cells used in solution.
    SKGraph *        mGraph;
    const QList<int> *     mPossibilities;
    const QList<int> *     mPossibilitiesIndex;

    // Print the current state of the Sudoku puzzle.
    void printSudoku();

    // Print DLX matrix (default is to skip printing those that are too large).
    void      printDLX (bool forced = false);
};

#endif // DLXSOLVER_H
