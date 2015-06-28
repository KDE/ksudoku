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

#include "mathdokugenerator.h"
#include "skgraph.h"
#include "cagegenerator.h"

#include <QDebug>

MathdokuGenerator::MathdokuGenerator (SKGraph * graph)
    :
    mGraph       (graph)
{
}

bool MathdokuGenerator::generateMathdokuTypes (BoardContents & puzzle,
                                               BoardContents & solution,
                                               Difficulty difficultyRequired)
{
    // Cage sizes must be no more than the number of cells in a column or row.
    int  maxSize   = qMin ((2 + difficultyRequired), mGraph->order());
    int  maxVal    = 1000;
    bool hideOps   = false;
    // int  maxCombos = 120;
    int  maxCombos = 2000;

    int  maxTries  = 20;

    CageGenerator cageGen (solution);

    int  numTries = 0;
    int  numMultis = 0;
    int  n = 0;
    while ((n <= 0) && (numTries < maxTries)) {
	numTries++;
	n = cageGen.makeCages (mGraph, maxSize, maxVal, hideOps, maxCombos);
	if (n < 0) {
	    numMultis++;
	}
    }
    if (numTries >= maxTries) {
	qDebug() << "makeCages() FAILED after" << numTries << "tries"
	         << numMultis << "multi-solutions";
        return false;		// Try another set of Sudoku cell-values.
    }

    qDebug() << "makeCages() required" << numTries << "tries"
             << numMultis << "multi-solutions";;
    puzzle = mGraph->emptyBoard();
    for (int n = 0; n < mGraph->cageCount(); n++) {
         if (mGraph->cage(n).count() == 1) {
             int index = mGraph->cage(n).at(0);
             puzzle[index] = solution.at(index);
         }
    }
    return true;
}

#include "mathdokugenerator.moc"
