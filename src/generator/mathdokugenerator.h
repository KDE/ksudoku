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

#ifndef MATHDOKUGENERATOR_H
#define MATHDOKUGENERATOR_H

#include "globals.h"

#include <QVector>

class SKGraph;

/**
 * @class MathdokuGenerator
 * @short Generator for Killer Sudoku and Mathdoku puzzles.
 *
 * Generates a Killer Sudoku or Mathdoku puzzle from a Latin Square that
 * satisfies Sudoku-type constraints. It acts as a controller for makeCages().
 */
class MathdokuGenerator
{
public:
    MathdokuGenerator (SKGraph * graph);

    /**
     * Generate a Mathdoku or Killer Sudoku puzzle.
     *
     * @param puzzle             The generated puzzle (returned).
     * @param solution           The values that must go into the solution.
     * @param solutionMoves      A pointer that returns an ordered list of cells
     *                           found by the solver when it reached a solution.
     * @param difficultyRequired The requested level of difficulty.
     *
     * @return                   True if puzzle-generation succeeded, false
     *                           if too many tries were required.
     */
    bool generateMathdokuTypes (BoardContents & puzzle,
                                BoardContents & solution,
                                QList<int> * solutionMoves,
                                Difficulty difficultyRequired);

    /**
     * Solve a Mathdoku or Killer Sudoku and check how many solutions there are.
     * The solver requires only the puzzle-graph, which contains all the cages.
     *
     * @param solution           The values returned as the solution.
     * @param solutionMoves      A pointer that returns an ordered list of cells
     *                           found by the solver when it reached a solution.
     *
     * @return                   0  = there is no solution,
     *                           1  = there is a unique solution,
     *                           >1 = there is more than one solution.
     */
    int solveMathdokuTypes     (BoardContents & solution,
                                QList<int> * solutionMoves);

private:
    SKGraph *  mGraph;		// The layout, rules and geometry of the puzzle.
};

#endif // MATHDOKUGENERATOR_H
