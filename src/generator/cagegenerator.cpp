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

#include "globals.h"
#include "skgraph.h"
#include "dlxsolver.h"
#include "cagegenerator.h"

#include <QDebug>

// #define MATHDOKU_LOG

CageGenerator::CageGenerator (const BoardContents & solution)
    :
    mSolution (solution)
{
    mMinSingles = 2;
    mMaxSingles = 4;

    mDLXSolver = new DLXSolver (this);	// Auto-deleted by QObject.
    mPossibilities = new QList<int>();
    mPossibilitiesIndex = new QList<int>();
}

CageGenerator::~CageGenerator()
{
    mPossibilities->clear();
    mPossibilitiesIndex->clear();
    delete mPossibilities;
    delete mPossibilitiesIndex;
}

int CageGenerator::makeCages (SKGraph * graph, int maxSize, int maxValue,
                              bool hideOperators, int maxCombos)
{
    // TODO - Use maxValue when OK'ing cages(?).
    // TODO - Experiment with mMinSingles and mMaxSingles. Make them parameters?

    QList<int> saveUnusedCells;
    QList<int> saveNeighbourFlags;
#ifdef MATHDOKU_LOG
    QVector<char> usedCells;
#endif

    mMaxCombos = maxCombos;
    init (graph, hideOperators);
    graph->clearCages();

    mPossibilities->clear();
    mPossibilitiesIndex->clear();
    mPossibilitiesIndex->append(0);
    mSingles = 0;

#ifdef MATHDOKU_LOG
    usedCells.fill ('-', mBoardArea);
    qDebug() << "USED CELLS     " << usedCells;
#endif

    // TODO - Will probably need to limit the number of size-1 cages and maybe
    //        guarantee a minimum number as well.
    // TODO - Might need to generate at least one maximum-size cage first..

    /*
    1. Select the starting point and size for a cage.

       Top priority for a starting point is a cell that is surrounded on three
       or four sides, otherwise the cell is chosen at random from the list of
       unused cells.s

       A cell surrounded on four sides must become a single-cell cage, with a
       pre-determined value and no operator. Choosing a cell surrounded on three
       sides allows the cage to occupy and grow out of tight corners, avoiding
       an excess of small and single-cell cages.

       The chosen size is initially 1 (needed to control the difficulty of the
       puzzle) and later a random number from 2 to the maximum cage-size. The
       maximum cage-size also affects difficulty.

    2. Use the function makeOneCage() to make a cage of the required size.

       The makeOneCage() function keeps adding unused neighbouring cells until
       the required size is reached or it runs out of space to grow the cage
       further. It updates the lists of used cells and neighbours as it goes.
       A neighbour that would otherwise become surrounded on all four sides
       is always added to the cage as it grows, but normally the next cell is
       chosen randomly.

    3. Use the function setCageTarget() to choose an operator (+*-/), calculate
       the cage's value from the cell-values in the puzzle's solution and find
       all the possible combinations of values that cells in the cage *might*
       have, as seen by the user.

       The possible combinations are used when solving the generated puzzle,
       using the DLX algorithm, to check that the puzzle has a unique solution.
       Many generated puzzles have multiple solutions and have to be discarded.

    4. Validate the cage, using function isCageOK().

       A cage can be rejected if it might make the puzzle too hard or too easy.
       If so, discard the cage, back up and repeat steps 1 to 4.

    5. Repeat steps 1 to 4 until all cells have been assigned to cages.
    */
    int numFailures = 0;
    int maxFailures = 20;

    while (mUnusedCells.count() > 0) {
	QVector<int> cage;
	CageOperator cageOperator;
	int          cageValue;
	int          chosenSize;
	int          index = -1;
	Q_FOREACH (int n, mUnusedCells) {
	    switch (mNeighbourFlags.at (n)){
	    case 7:
	    case 11:
	    case 13:
	    case 14:
		index = n;		// Enclosed on three sides: start here.
		chosenSize = qrand() % (maxSize - 1) + 2;
#ifdef MATHDOKU_LOG
		qDebug() << "CHOSE CUL-DE-SAC" << mNeighbourFlags.at (n)
		       << "at" << index;
#endif
		break;
	    case 15:
		index = n;		// Isolated cell: size 1 is forced.
		chosenSize = 1;
#ifdef MATHDOKU_LOG
		qDebug() << "CHOSE ISOLATED" << mNeighbourFlags.at (n)
		       << "at" << index;
#endif
		break;
	    default:
		break;
	    }
	    if (index >= 0) {
		break;
	    }
	}
	if (index < 0) {		// Pick a starting cell at random.
	    index = mUnusedCells.at (qrand() % mUnusedCells.count());
	    if (mSingles < mMinSingles) {
		chosenSize = 1;				// Start with size 1.
		mSingles++;
#ifdef MATHDOKU_LOG
		qDebug() << "INITIAL SINGLE" << mSingles;
#endif
	    }
	    else {
		chosenSize = qrand()%(maxSize - 1) + 2;	// Then avoid size 1.
	    }
	}
#ifdef MATHDOKU_LOG
	qDebug() << "chosenSize" << chosenSize << "cell" << index;
#endif

	saveUnusedCells = mUnusedCells;
	saveNeighbourFlags = mNeighbourFlags;
#ifdef MATHDOKU_LOG
	qDebug() << "CALL makeOneCage: index" << index << "size" << chosenSize;
#endif

	cage = makeOneCage (index, chosenSize);
	setCageTarget (cage, cageOperator, cageValue);
	if (! cageIsOK (cage, cageOperator, cageValue)) {
	    mUnusedCells = saveUnusedCells;
	    mNeighbourFlags = saveNeighbourFlags;
#ifdef MATHDOKU_LOG
	    qDebug() << "CAGE IS NOT OK - unused" << mUnusedCells << "\n";
#endif
	    numFailures++;
	    if (numFailures >= maxFailures) {
		qDebug() << "makeOneCage() HAD" << numFailures << "failures"
		         << "maximum is" << maxFailures;
		return 0;	// Too many problems with making cages.
	    }
	    continue;
	}

	// The cage is OK: add it to the puzzle's layout.
	mGraph->addCage (cage, cageOperator, cageValue);

#ifdef MATHDOKU_LOG
	qDebug() << "CAGE" << mGraph->cageCount() << cage;
	char tag = 'a' + mGraph->cageCount() - 1;
	Q_FOREACH (int cell, cage) {
	    usedCells[cell] = tag;
	}
	qDebug() << "LAYOUT" << tag << usedCells << "\n";
	for (int row = 0; row < mOrder; row++) {
	    for (int col = 0; col < mOrder; col++) {
		fprintf (stderr, "%c ", usedCells.at (col * mOrder + row));
	    }
	    fprintf (stderr, "\n");
	}
	fprintf (stderr, "\n");
#endif
	QList<int> flagsList;
	Q_FOREACH (int cell, mUnusedCells) {
	    flagsList.append (mNeighbourFlags.at (cell));
	}
#ifdef MATHDOKU_LOG
	qDebug() << "FLAGS" << flagsList;
	qDebug() << "UNUSED" << mUnusedCells << "\n";
#endif
    }
    int nCages = mPossibilitiesIndex->count() - 1;
#ifdef MATHDOKU_LOG
    qDebug() << "Cages in mPossibilitiesIndex" << nCages
             << "generated cages" << mGraph->cageCount();
#endif
    int totCombos = 0;
    for (int n = 0; n < nCages; n++) {
	int nVals = mPossibilitiesIndex->at (n+1) - mPossibilitiesIndex->at (n);
	int size = mGraph->cage (n).size();
	int nCombos =  nVals / size;
#ifdef MATHDOKU_LOG
	qDebug() << "Cage" << n << "values" << nVals << "size" << size
                 << "combos" << nCombos;
#endif
	totCombos += nCombos;
    }
#ifdef MATHDOKU_LOG
    qDebug() << "TOTAL COMBOS" << totCombos << "\n";
#endif

    // Use the DLX solver to check if this puzzle has a unique solution.
    int nSolutions = mDLXSolver->solveMathdoku (mGraph, mPossibilities,
                                                mPossibilitiesIndex, 2);
    if (nSolutions == 0) {
	qDebug() << "FAILED TO FIND A SOLUTION: nSolutions =" << nSolutions;
	return -1;		// At least one solution must exist.
    }
    else if (nSolutions > 1) {
	qDebug() << "NO UNIQUE SOLUTION: nSolutions =" << nSolutions;
	return -1;		// There must only one solution.
    }

    qDebug() << "UNIQUE SOLUTION FOUND: nSolutions =" << nSolutions;
    return mGraph->cageCount();
}

QVector<int> CageGenerator::makeOneCage (int seedCell, int requiredSize)
{
    QVector<int> cage;
    QList<int>   unusedNeighbours;
    const int    direction[4] = {N, E, S, W};
    const int    increment[4] = {-1, +mOrder, +1, -mOrder};
    const int    opposite[4]  = {S, W, N, E};
    int          index = seedCell;

    cage.clear();
    unusedNeighbours.clear();
    unusedNeighbours.append (index);

    while (true) {
	// Update the chosen cell's neighbours.
	int flags = mNeighbourFlags.at (index);
	for (int k = 0; k < 4; k++) {
	    if (flags & direction[k]) {
		continue;		// Already flagged.
	    }
	    int nb = index + increment[k];
	    mNeighbourFlags[nb] = mNeighbourFlags[nb] | opposite[k];
	    if (mUnusedCells.indexOf (nb) >= 0) {
		unusedNeighbours.append (nb);
	    }
	}
#ifdef MATHDOKU_LOG
	qDebug() << "index" << index << "NEIGHBOURS" << unusedNeighbours;
#endif
	mUnusedCells.removeAt (mUnusedCells.indexOf (index));
	mNeighbourFlags[index] = TAKEN;
	cage.append (index);
	if (cage.size() >= requiredSize) {
	    break;
	}

	int unb = unusedNeighbours.indexOf (index);
	while (unb >= 0) {
	    unusedNeighbours.removeAt (unb);
	    unb = unusedNeighbours.indexOf (index);
	}
	if (unusedNeighbours.isEmpty()) {
	    break;
	}

	// Pick a neighbour to be added to the cage.
	index = -1;
	Q_FOREACH (unb, unusedNeighbours) {
            flags = mNeighbourFlags.at (unb);
	    if (flags == 15) {
		// Choose a cell that has been surrounded and isolated.
		index = unb;
		break;
	    }
	}
	if (index < 0) {
	    // Otherwise, choose a neighbouring cell at random.
	    unb = qrand() % unusedNeighbours.count();
	    index = unusedNeighbours.at (unb);
	}
    }
    return cage;
}

void CageGenerator::setCageTarget (QVector<int> cage,
                                   CageOperator & cageOperator,
                                   int & cageValue)
{
    CageOperator op;
    int value = 0;
    int size  = cage.size();
    QVector<int> digits;
    digits.fill (0, size);
#ifdef MATHDOKU_LOG
    qDebug() << "CAGE SIZE" << size << "CONTENTS" << cage;
#endif
    for (int n = 0; n < size; n++) {
	int digit = mSolution.at (cage.at(n));
	digits[n] = digit;
#ifdef MATHDOKU_LOG
	qDebug() << "Add cell" << cage.at(n)
		 << "value" << mSolution.at (cage.at(n))
		 << (n + 1) << "cells"; // : total" << value;
#endif
    }
    if (size == 1) {
	cageOperator = NoOperator;
	cageValue = digits[0];
#ifdef MATHDOKU_LOG
	qDebug() << "SINGLE CELL: #" << mSingles << "val" << cageValue;
#endif
	return;
    }

    int lo = 0;
    int hi = 0;
    if (mKillerSudoku) {
	// Killer Sudoku has an Add operator for every calculated cage.
	op = Add;
    }
    else {
	// Mathdoku has a randomly chosen operator for each calculated cage.
	int weights[4]      = {50, 30, 15, 15};
	CageOperator ops[4] = {Divide, Subtract, Multiply, Add};
	if (size != 2) {
	    weights[0] = weights[1] = 0;
	}
	else {
	    lo = qMin (digits[0], digits[1]);
	    hi = qMax (digits[0], digits[1]);
	    weights[0] = ((hi % lo) == 0) ? 50 : 0;
	}

	int roll = qrand() % (weights[0]+weights[1]+weights[2]+weights[3]);
#ifdef MATHDOKU_LOG
	int wTotal = (weights[0]+weights[1]+weights[2]+weights[3]);
	qDebug() << "ROLL" << roll << "VERSUS" << wTotal << "WEIGHTS"
		 << weights[0] << weights[1] << weights[2] << weights[3];
#endif
	int n = 0;
	while (n < 4) {
	    roll = roll - weights[n];
	    if (roll < 0) {
		break;
	    }
	    n++;
	}
	op = ops[n];
    }

    switch (op) {
    case Divide:
	value = hi / lo;
	break;
    case Subtract:
	value = hi - lo;
	break;
    case Multiply:
	value = 1;
	for (int i = 0; i < size; i++) {
	    value = value * digits[i];
	}
	break;
    case Add:
	value = 0;
	for (int i = 0; i < size; i++) {
	    value = value + digits[i];
	}
	break;
    default:
	break;
    }
#ifdef MATHDOKU_LOG
    qDebug() << "CHOSE OPERATOR" << op << "value" << value << digits;
#endif
    cageOperator = op;
    cageValue = value;
}

bool CageGenerator::cageIsOK (const QVector<int> cage,
                              CageOperator cageOperator, int cageValue)
{
    // TODO - Is it worth checking for duplicate digits in Mathdoku, before
    //        going the whole hog and checking for constraint satisfaction?
    // NOTE - The solution, by definition, has to satisfy constraints, even
    //        if it does have duplicate digits (ie. those digits must be in
    //        different rows/columns/boxes).

    int nDigits = cage.size();

    // In Killer Sudoku, the cage's solution must not contain duplicate digits.
    if (mKillerSudoku) {
	QVector<bool> usedDigits;
	usedDigits.fill (false, mOrder + 1);	// Digits' range is 1->order.
	for (int n = 0; n < nDigits; n++) {
	    int digit = mSolution.at (cage.at(n));
	    if (usedDigits.at (digit)) {
		qDebug() << "SOLUTION VALUES CONTAIN DUPLICATE" << digit;
		return false;			// Cannot use this cage.
	    }
	    usedDigits[digit] = true;
	}
    }

    // Get all possibilities and keep checking, as we go, that the cage is OK.
    bool isOK = true;
    if ((nDigits > 1) && mHiddenOperators && (! mKillerSudoku)) {
	// Operators are hidden, so we must consider every possible operator.
	if (nDigits == 2) {
	    setPossibilities (cage, Divide, cageValue);
	    setPossibilities (cage, Subtract, cageValue);
	}
        setPossibilities (cage, Add, cageValue);
        setPossibilities (cage, Multiply, cageValue);
    }
    else {
	// Operators are visible/fixed, so we can consider fewer possibilities.
	setPossibilities (cage, cageOperator, cageValue);
    }
    int numPoss = (mPossibilities->size() - mPossibilitiesIndex->last());

    // There should be some possibilities and not too many (wrt difficulty).
    isOK &= (numPoss > 0);
    isOK &= ((numPoss / nDigits) <= mMaxCombos);

    if (isOK) {
	// Save the possibilities, for use when testing the puzzle solution.
        mPossibilitiesIndex->append (mPossibilities->size());
    }
    else {
	// Discard the possibilities: this cage is no good.
	qDebug() << "CAGE REJECTED: combos" << (numPoss / nDigits)
                 << "max" << mMaxCombos << cage << cageValue << cageOperator;
	while (numPoss > 0) {
	    mPossibilities->removeLast();
	    numPoss--;
        }
    }
    return isOK;
}

void CageGenerator::setPossibilities (const QVector<int> cage,
                                      CageOperator cageOperator, int cageValue)
{
    // Generate sets of possible solution-values from the range 1 to mOrder.
    switch (cageOperator) {
    case NoOperator:
	mPossibilities->append (cageValue);
	break;
    case Add:	
    case Multiply:
	setPossibleAddsOrMultiplies (cage, cageOperator, cageValue);
	break;
    case Divide:
	for (int a = 1; a <= mOrder; a++) {
	    for (int b = 1; b <= mOrder; b++) {
		if ((a == b * cageValue) || (b == a * cageValue)) {
		    *mPossibilities << a << b;
		}
	    }
	}
	break;
    case Subtract:
	for (int a = 1; a <= mOrder; a++) {
	    for (int b = 1; b <= mOrder; b++) {
		if (((a - b) == cageValue) || ((b - a) == cageValue)) {
		    *mPossibilities << a << b;
		}
	    }
	}
	break;
    }
}

void CageGenerator::setPossibleAddsOrMultiplies
        (const QVector<int> cage, CageOperator cageOperator, int requiredValue)
{
    QVector<int> digits;
    int maxDigit = mOrder;
    int nDigits = cage.size();
    int currentValue;
    int nTarg = 0;
    int nDupl = 0;
    int nCons = 0;
    int loopCount = 1;

    // Calculate the number of possible sets of digits in the cage.
    for (int n = 0; n < nDigits; n++) {
	loopCount = loopCount * maxDigit;
    }

    // Start with a sum or product of all 1's, then check all possibilities.
    digits.fill (1, nDigits);
    currentValue = (cageOperator == Add) ? nDigits : 1;
    for (int n = 0; n < loopCount; n++) {
	if (currentValue == requiredValue) {
	    nTarg++;
#ifdef MATHDOKU_LOG
	    qDebug() << "TARGET REACHED" << requiredValue
		     << "OP" << cageOperator << "DIGITS" << digits;
	    if (hasDuplicates (nDigits, digits)) {
		nDupl++;
	    }
	    qDebug() << "CONTAINS DUPLICATE(S)"
		     << hasDuplicates (nDigits, digits);
#endif
	    if (isSelfConsistent (cage, nDigits, digits)) {
		// Include only the possibilities that satisfy Sudoku rules.
		mPossibilities->append (QList<int>::fromVector (digits));
		nCons++;
	    }
#ifdef MATHDOKU_LOG
	    qDebug() << "SELF CONSISTENT"
		     << isSelfConsistent (cage, nDigits, digits)
		     << digits << "cage" << cage;
#endif
	}

	// Calculate the next set of possible digits (as in an odometer).
	for (int d = 0; d < nDigits; d++) {
	    digits[d]++;
	    if (digits.at (d) > maxDigit) {	// Carry 1.
		digits[d]  -= maxDigit;
	    }
	    else {
		break;				// No carry.
	    }
	}

	// Calculate the sum or product of this set of digits.
	currentValue = (cageOperator == Add) ? 0 : 1;
	for (int d = 0; d < nDigits; d++) {
	    if (cageOperator == Add) {
		currentValue += digits[d];
	    }
	    else {
		currentValue *= digits[d];
	    }
	}
    }
#ifdef MATHDOKU_LOG
    qDebug() << loopCount << "possibles" << nTarg << "hit target"
	     << nDupl << "had duplicate(s)" << nCons << "consistent";
#endif
}

bool CageGenerator::hasDuplicates (int nDigits, QVector<int> digits)
{
    QVector<bool> usedDigits;
    usedDigits.fill (false, mOrder + 1);	// Digits range is 1->order.
    for (int n = 0; n < nDigits; n++) {
	int digit = digits.at (n);
	if (usedDigits.at (digit)) {
	    return true;
	}
	usedDigits[digit] = true;
    }
    return false;
}

bool CageGenerator::isSelfConsistent (const QVector<int> cage,
                                      int nDigits, QVector<int> digits)
{
    QVector<int> usedGroups;
    usedGroups.fill (0, mGraph->cliqueCount());
    for (int n = 0; n < nDigits; n++) {
	int cell = cage.at (n);
	int digit = digits.at (n);
	int mask = 1 << digit;
	QList<int> groupList = mGraph->cliqueList (cell);
	Q_FOREACH (int group, groupList) {
	    // qDebug() << "Digit" << (n + 1) << "value" << digit << "mask"
		     // << mask << "position" << cell << "conflict group"
		     // << group << "cells" << mGraph->clique (group)
		     // << "group mask" << usedGroups.at (group);
	    if (mask & usedGroups.at (group)) {
		return false;
	    }
	    usedGroups [group] |= mask;
	}
    }
    return true;
}

void CageGenerator::init (SKGraph * graph, bool hideOperators)
{
    /* INITIAL SETUP FOR THE CageGenerator.

    1. Mark all cells as unused (for cages). This is represented by a list of
       unused cells (QList<int>) containing cell-indices.
    2. Make a list showing blocked sides (NSWE bitmap) of each cell. A blocked
       side means that there is a cell assigned to a cage on that side. Cells
       at the edges or corners of the board are set up to have imaginary (dummy)
       cages as neighbours.
    */
    mGraph     = graph;
    mOrder     = graph->order();
    mBoardArea = mOrder * mOrder;

    mKillerSudoku    = (graph->specificType() == KillerSudoku);
    mHiddenOperators = mKillerSudoku ? true : hideOperators;
    qDebug() << "\nMAKE CAGES init(): HIDDEN OPERATORS" << mHiddenOperators;

    mUnusedCells.clear();
    mNeighbourFlags.clear();

    for (int n = 0; n < mBoardArea; n++) {
        mUnusedCells.append (n);

        int col            = graph->cellPosX (n);
        int row            = graph->cellPosY (n);
        int limit          = mOrder - 1;
        int neighbours     = ALONE;

        // Mark cells on the perimeter of the board as having dummy neighbours.
        if (row == 0) {
            neighbours = neighbours | N; // Cell to the North is unavailable.
        }
        if (row == limit) {
            neighbours = neighbours | S; // Cell to the South is unavailable.
        }
        if (col == 0) {
            neighbours = neighbours | W; // Cell to the West is unavailable.
        }
        if (col == limit) {
            neighbours = neighbours | E; // Cell to the East is unavailable.
        }

        mNeighbourFlags.append (neighbours);
    }
#ifdef MATHDOKU_LOG
    qDebug() << "UNUSED CELLS   " << mUnusedCells;
    qDebug() << "NEIGHBOUR-FLAGS" << mNeighbourFlags;
#endif
}

#include "cagegenerator.moc"