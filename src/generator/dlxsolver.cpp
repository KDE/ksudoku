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

#include <QDebug> // IDW test.
#include "stdio.h" // IDW test.

DLXSolver::DLXSolver (QObject * parent)
    :
    QObject       (parent),
    mBoardValues  (0),
    mGraph        (0)
{
    qDebug() << "DLXSolver constructor entered";
    mCorner = new DLXNode;
    clear();
}

DLXSolver::~DLXSolver()
{
    qDebug() << "DLXSolver destructor entered";
    deleteAll();
    delete mCorner;
}

void DLXSolver::printDLX (bool forced)
{
    if (!forced || (mGraph->order() > 5)) {
        return;			// Skip large printouts, unless forced.
    }

    if ((mEndNodeNum < 0) || (mEndRowNum < 0) || (mEndColNum < 0)) {
        fprintf (stderr, "\nDLXSolver::print(): EMPTY, mEndNodeNum %d, "
                "mEndRowNum %d, mEndColNum %d\n\n",
                mEndNodeNum, mEndRowNum, mEndColNum);
        return;
    }
    fprintf (stderr, "\nDLX Matrix has %d cols, %d rows and %d ones\n\n",
                mEndColNum + 1, mEndRowNum + 1, mEndNodeNum + 1);
    DLXNode * colDLX = mCorner->right;
    if (colDLX == mCorner) {
        fprintf (stderr, "ALL COLUMNS ARE HIDDEN\n");
    }
    int lastCol = -1;
    while (colDLX != mCorner) {
        int col = mColumns.indexOf(colDLX);
    // for (int col = 0; col <= mEndColNum; col++) {
        fprintf (stderr, "Col %02d, %02d rows  ", col, mColumns.at(col)->value);
        DLXNode * node = mColumns.at(col)->below;
        while (node != mColumns.at(col)) {
            fprintf (stderr, "%02d ", node->value);
            node = node->below;
        }
        int gap = col - (lastCol + 1);
        if (gap > 0) {
            fprintf (stderr, "covered %02d", gap);
        }
        fprintf (stderr, "\n");
        colDLX = colDLX->right;
        lastCol = col;
    }
    fprintf (stderr, "\n");
}

void DLXSolver::recordSolution (const int solutionNum, QList<DLXNode *> & solution)
{
    int order = mGraph->order();
    Q_FOREACH (DLXNode * node, solution) {
        int rowNumDLX = node->value;
        int cellValue = (rowNumDLX % order) + 1;
        int position  = rowNumDLX /order;
        // int row       = position / order;
        // int col       = position % order;
        // mBoardValues [col*order + row] = cellValue;
        mBoardValues [position] = cellValue;
    }

    fprintf (stderr, "\nSOLUTION %d\n\n", solutionNum);

    printSudoku();
}

void DLXSolver::printSudoku() // (const BoardContents & boardValues)
{
    // TODO - The code at SudokuBoard::print() is VERY similar...

    // Used for test and debug, but the format is also parsable and loadable.

    char nLabels[] = "123456789";
    char aLabels[] = "abcdefghijklmnopqrstuvwxy";
    int index, value;
    int order     = mGraph->order();
    int blockSize = mGraph->base();
    int sizeX     = mGraph->sizeX();
    int sizeY     = mGraph->sizeY();
    int sizeZ     = mGraph->sizeZ();	// If 2-D, depth == 1, else depth > 1.

    for (int k = 0; k < sizeZ; k++) {
      int z = (sizeZ > 1) ? (sizeZ - k - 1) : k;
      for (int j = 0; j < sizeY; j++) {
        if ((j != 0) && (j % blockSize == 0)) {
            fprintf (stderr, "\n");	// Gap between square blocks.
        }
        int y = (sizeZ > 1) ? (sizeY - j - 1) : j;
        for (int x = 0; x < sizeX; x++) {
            index = mGraph->cellIndex (x, y, z);
            value = mBoardValues.at (index);
            if (x % blockSize == 0) {
                fprintf (stderr, "  ");	// Gap between square blocks.
            }
            if (value == UNUSABLE) {
                fprintf (stderr, " '");	// Unused cell (e.g. in Samurai).
            }
            else if (value == VACANT) {
                fprintf (stderr, " -");	// Empty cell (to be solved).
            }
            else {
                value--;
                char label = (order > 9) ? aLabels[value] : nLabels[value];
                fprintf (stderr, " %c", label);	// Given cell (or clue).
            }
        }
        fprintf (stderr, "\n");		// End of row.
      }
      fprintf (stderr, "\n");		// Next Z or end of 2D puzzle/solution.
    }
}

int DLXSolver::solveSudoku (SKGraph * graph, const BoardContents & boardValues,
                                               int solutionLimit)
{
    // TODO - Internal procedures buildDLXMatrix(), setKnownValues().
    fprintf (stderr, "\nTEST for DLXSolver\n\n");

    mBoardValues     = boardValues;	// Used later for filling in a solution.
    mGraph           = graph;

    int nSolutions   = 0;
    int order        = graph->order();
    int boardArea    = graph->size();
    int nGroups      = graph->cliqueCount();
    int vacant       = VACANT;
    int unusable     = UNUSABLE;

    printSudoku();			// IDW test.

    clear();				// Empty the DLX matrix.

    // Generate a DLX matrix for an empty Sudoku grid of the required type.
    // It has (boardArea*order) rows and (boardArea + nGroups*order) columns.
    qDebug() << "DLXSolver::solve: Order" << order << "boardArea" << boardArea
             << "nGroups" << nGroups;
    int rowNumDLX = 0;
    for (int index = 0; index < boardArea; index++) {
        int row    = graph->cellPosY (index);
        int col    = graph->cellPosX (index);
        if (boardValues.at(index) == unusable) {	// e.g. For Samurai.
            qDebug() << "    Index" << index << "row" << row << "col" << col
                     << "UNUSABLE";
            // Add dummy rows, so rowNumDLX always == index*order + value - 1.
            rowNumDLX = rowNumDLX + order;
            continue;
        }
        // Get a list of groups (row, column, etc.) that contain this cell.
        QList<int> groups = graph->cliqueList (index);
        qDebug() << "    Index" << index << "row" << row << "col" << col
                 << "groups" << groups;

        // Generate a row for each possible value of this cell in the Sudoku
        // grid, representing part of a possible solution. Each row must have
        // 1's in columns that correspond to a constraint on the cell and on the
        // value (in each group to which the cell belongs --- row, column, etc).

        for (int possValue = 0; possValue < order; possValue++) {
            addNode (rowNumDLX, index);		// Mark cell fill-in constraint.
            Q_FOREACH (int group, groups) {
                // Mark possibly-satisfied constraints for row, column, etc.
                addNode (rowNumDLX, boardArea + group*order + possValue);
            }
            rowNumDLX++;
        }
    }
    printDLX();

    // Eliminate clues and unusable cells from the DLX matrix.
    int n = 0;
    for (int index = 0; index < boardArea; index++) {
        int value = boardValues.at(index);
        if ((value == vacant) || (value == unusable)) {
            if (value == unusable) {
                // Cover the empty column: no value has to be entered here.
                coverColumn (mColumns.at (index));
            }
            continue;
        }
        // Non-empty cell: eliminate the corresponding DLX row and columns.
        rowNumDLX = index*order + value - 1;
        qDebug() << "CLUE AT INDEX" << index << "value" << value
                 << "ELIMINATE DLX ROW" << rowNumDLX;
        DLXNode * start = mRows.at(rowNumDLX);
        DLXNode * colDLX = start;
        do {
            qDebug() << "Cover DLX column"
                     << mColumns.indexOf(colDLX->columnHeader);
            coverColumn (colDLX->columnHeader);
            colDLX = colDLX->right;
        } while (colDLX != start);
        n++;
        if (n >= 3) {
            // row = order; // /////////////// ??????????????????????????????
            // break; // ///////////////////// ??????????????????????????????
        }
    }
    printDLX (true);
    // return 0; // IDW test.

    fprintf (stderr, "\nMatrix had %d rows, %d columns and %d ones\n",
            mRows.count(), mColumns.count(), mRows.count()*4);
    int nRowsLeft = 0;
    int nColsLeft = 0;
    int nNodes    = 0;
    QVector<DLXNode *> rowsRemaining;
    rowsRemaining.fill (0, mRows.count());
    DLXNode * colDLX = mCorner->right;
    while (colDLX != mCorner) {
        nColsLeft++;
        DLXNode * node = colDLX->below;
        while (node != colDLX) {
            int rowNum = node->value;
            if (rowsRemaining.at (rowNum) == 0) {
                nRowsLeft++;
            }
            rowsRemaining[rowNum] = mRows.at (rowNum);
            nNodes++;
            node = node->below;
        }
        colDLX = colDLX->right;
    }

    // Print a map of the (sparse) DLX matrix.
    fprintf (stderr, "Matrix NOW has %d rows, %d columns and %d ones\n\n",
            nRowsLeft, nColsLeft, nNodes);
    int rowNum = 0;
    Q_FOREACH (DLXNode * r, rowsRemaining) {
        if (nColsLeft > 64) {
            break;			// Would take two or more lines per row.
        }
        fprintf (stderr, "%3d ", rowNum);
        rowNum++;
        if (r == 0) {
            fprintf (stderr, "\n");	// Empty line: row was removed.
            continue;
        }
        QVector<char> colPrint;
        colPrint.fill (' ', mColumns.count());
        DLXNode * c = mCorner->right;
        while (c != mCorner) {
            colPrint [mColumns.indexOf(c)] = '.';	// Column is present.
            c = c->right;
        }
        c = r;
        do {
            colPrint [mColumns.indexOf(c->columnHeader)] = '1';	// 1 in row.
            c = c->right;
        } while (c != r);

        for (int n = 0; n < colPrint.count(); n++) {
            if (n%8 == 0) {
                fprintf (stderr, "|");
            }
            fprintf (stderr, "%c", colPrint.at (n));
        }
        fprintf (stderr, "|\n");
    }

    // Solve the DLX-matrix equivalent of the Sudoku-style puzzle.
    qDebug() << "\nCALL solveDLX(), solution limit" << solutionLimit;
    nSolutions = solveDLX (solutionLimit);
    qDebug() << "FOUND" << nSolutions << "solutions, limit" << solutionLimit;
    return nSolutions;
}

void DLXSolver::testDLX ()
{
    const int test [6][7] = {
        {1,0,0,1,0,0,1},
        {1,0,0,1,0,0,0},
        {0,0,0,1,1,0,1},
        {0,0,1,0,1,1,0},
        {0,1,1,0,0,1,1},
        {0,1,0,0,0,0,1}
    };
    const int w = 7;
    const int h = 6;
    fprintf (stderr, "\nTEST MATRIX\n");
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            fprintf (stderr, " %d", test[row][col]);
        }
        fprintf (stderr, "\n");
    }
    fprintf (stderr, "\n");
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            if (test[row][col]) addNode (row, col);
        }
    }
    printDLX();
    solveDLX (0);
}

/*        HOW THE DANCING LINKS ALGORITHM WORKS IN METHOD solveDLX().

     The solution algorithm must satisfy every column's constraint if it is to
     succeed. So it proceeds by taking one column at a time and "covering" it.
     In this context, "covering" can mean logically hiding the column and so
     reducing the size of the matrix to be solved, as happens in the method
     coverColumn() below. But it also means that the constraint represented
     by that column is "covered" or satisfied, in the sense that a payment
     of money can be "covered" (i.e. provided for).

     Whichever column is covered first, one of its non-zero values must be in
     a row that is part of the solution, always supposing that there is a
     solution. Knuth recommends to select first the columns that have the
     smallest number of 1's. The algorithm then tries each column in turn and
     each non-zero item within that column, backtracking if there are no items
     left in a column. When all columns have been covered, a solution has been
     found, but the algorithm continues to search for other solutions until
     the caller's solution limit is reached. The solutions are delivered via
     the Qt library's signal-slot mechanism.

     The algorithm terminates when the first column in the next solution is to
     be chosen, but one of the columns has no 1's in it, meaning that there can
     be no further solution. In principle, this can happen right at the start,
     because the corresponding problem is insoluble, but more likely will be
     after an extensive search where the solution limit parameter is zero (no
     limit) or the number of solutions found is less than the required limit or
     there is no solution, even after an extensive search. The algorithm then
     returns the integer number of solutions found (possibly 0).

     The "Dancing Links" aspect of the algorithm refers to lists of nodes that
     are linked in four directions (left, right, above and below) to represent
     columns, rows and column headers. Each node represents a 1 in the matrix.
     Covering a column involves unlinking it from the columns on each side of
     it and unlinking each row that has a 1 in that column from the rows above
     and below. One of the nodes in the column is included (tentatively) in
     the current partial solution and so the other nodes and their rows cannot
     be. At the same time, the matrix reduces to a sub-matrix and sub-problem.

     The thing is that the removed columns and rows "remember" their previous
     state and can easily be re-linked if backtracking becomes necessary and
     they need to be "uncovered". Thus the nodes, links, columns and rows can
     "dance" in and out of the matrix as the solution proceeds. Furthermore,
     the algorithm can be written without using recursion. It just needs to
     keep a LIFO list (i.e. a stack) of nodes tentatively included in the
     current solution. Using iteration should make the algorithm go fast.
*/

int DLXSolver::solveDLX (int solutionLimit)
{
    int solutionCount  = 0;
    int level          = 0;
    DLXNode * currNode = 0;
    DLXNode * bestCol  = 0;
    QList<DLXNode *> solution;
    bool searching     = true;
    bool descending    = true;

    if (mCorner->right == mCorner) {
        fprintf (stderr, "\nsolveDLX: EMPTY MATRIX, NOTHING TO SOLVE.\n\n");
        return solutionCount;
    }

    while (searching) {
        // Find the column with the least number of rows yet to be explored.
        int min = mCorner->right->value;
        bestCol = mCorner->right;
        for (DLXNode * p = mCorner->right; p != mCorner; p = p->right) {
            if (p->value < min) {
                bestCol = p;
                min = p->value;
            }
        }
        fprintf (stderr, "\nsolveDLX: BEST COLUMN %d level %d rows %d\n",
                 mColumns.indexOf (bestCol), level, bestCol->value);
        // diagnostic_1 (level, bestCol);	// Knuth p5, branching.

        coverColumn (bestCol);
        currNode = bestCol->below;
        solution.append (currNode);
        fprintf (stderr, "CURRENT SOLUTION: %d rows:", solution.size());
        Q_FOREACH (DLXNode * q, solution) {
            fprintf (stderr, " %d", q->value);
        }
        fprintf (stderr, "\n");

        while (descending) {
            if (currNode != bestCol) {
                // Found a row: cover all other cols of currNode's row (L to R).
                // Those constraints are satisfied (for now) by this row.
                DLXNode * p = currNode->right;
                while (p != currNode) {
                    coverColumn (p->columnHeader);
                    p = p->right;
                }
                printDLX(); // IDW test.

                if (mCorner->right != mCorner) {
                    break;	// Start searching a new sub-matrix.
                }
                // All columns covered: a solution has been found.
                solutionCount++;
                recordSolution (solutionCount, solution);
                if (solutionCount == solutionLimit) {
                    return solutionCount;
                }
            }
            else {
                // No more rows to try in this column.
                uncoverColumn (bestCol);
                if (level > 0) {
                    // Backtrack by one level.
                    solution.removeLast();
                    level--;
                    currNode = solution.at (level);
                    bestCol  = currNode->columnHeader;
                    qDebug() << "BACKTRACKED TO LEVEL" << level;
                }
                else {
                    // The search is complete.
                    return solutionCount;
                }
            }

            // Uncover all other columns of currNode's row (R to L).
            // Restores those constraints to unsatisfied state in reverse order.
            qDebug() << "RESTORE !!!";
            DLXNode * p = currNode->left;
            while (p != currNode) {
                uncoverColumn (p->columnHeader);
                p = p->left;
            }
            printDLX(); // IDW test.

            // Select next row down and continue searching for a solution.
            currNode = currNode->below;
            solution [level] = currNode;
            qDebug() << "SELECT ROW" << currNode->value
                     << "FROM COL" << mColumns.indexOf (currNode->columnHeader);
        } // End while (descending)

        level++;

    } // End while (searching)

    return solutionCount;		// Should never reach this point.
}

void DLXSolver::coverColumn (DLXNode * colDLX)
{
    fprintf (stderr, "coverColumn: %d rows %d\n",
             mColumns.indexOf(colDLX), colDLX->value); 
    colDLX->left->right = colDLX->right;
    colDLX->right->left = colDLX->left;

    DLXNode * colNode = colDLX->below;
    while (colNode != colDLX) {
        DLXNode * rowNode = colNode->right;
        qDebug() << "DLXSolver::coverColumn: remove DLX row" << rowNode->value;
        while (rowNode != colNode) {
            rowNode->below->above = rowNode->above;
            rowNode->above->below = rowNode->below;
            rowNode->columnHeader->value--;
            rowNode = rowNode->right;
        }
        colNode        = colNode->below;
    }
}

void DLXSolver::uncoverColumn (DLXNode * colDLX)
{
    DLXNode * colNode = colDLX->below;
    while (colNode != colDLX) {
        DLXNode * rowNode = colNode->right;
        qDebug() << "DLXSolver::uncoverColumn: return DLX row"
                 << rowNode->value;
        while (rowNode != colNode) {
            rowNode->below->above = rowNode;
            rowNode->above->below = rowNode;
            rowNode->columnHeader->value++;
            rowNode = rowNode->right;
        }
        colNode        = colNode->below;
    }

    fprintf (stderr, "uncoverColumn: %d rows %d\n",
             mColumns.indexOf(colDLX), colDLX->value); 
    colDLX->left->right = colDLX;
    colDLX->right->left = colDLX;
}

void DLXSolver::clear()
{
    qDebug() << "=============================================================";
    qDebug() << "DLXSolver::clear";

    // Logically clear the DLX matrix, but leave all previous nodes allocated.
    // This is to support faster DLX solving on second and subsequent puzzles.
    mEndNodeNum  = -1;
    mEndRowNum   = -1;
    mEndColNum   = -1;
    initNode (mCorner);
}

void DLXSolver::addNode (int rowNum, int colNum)
{
    // Get a node from the pool --- or create one.
    DLXNode * node   = getNode (rowNum, colNum);
    DLXNode * header = mColumns.at (colNum);

    // Circularly link the node onto the end of the row.
    if (mRows.at (rowNum) == 0) {
        mRows[rowNum] = node;	// First in row.
        initNode (node);	// Linked to itself at left and right.
    }
    else {
        addAtRight (node, mRows.at (rowNum));
    }


    // Circularly link the node onto the end of the column.
    addBelow (node, header);

    // Set the node's data-values.
    node->columnHeader = header;
    node->value        = rowNum;

    // Increment the count of nodes in the column.
    header->value++;
}

DLXNode * DLXSolver::getNode (int rowNum, int colNum)
{
    DLXNode * node;

    while (mEndColNum < colNum) {
        mEndColNum++;
       if (mEndColNum >= mColumns.count()) {
            // Allocate a column header only when needed, otherwise re-use one.
            mColumns.append (new DLXNode);
        }
        node = mColumns.at (mEndColNum);
        initNode (node);
        addAtRight (node, mCorner);
    }

    while (mEndRowNum < rowNum) {
        mEndRowNum++;
        if (mEndRowNum >= mRows.count()) {
            // Make space on the list if needed, otherwise re-use an entry.
            mRows.append (0);
        }
        else {
            mRows[mEndRowNum] = 0;
        }
    }

    mEndNodeNum++;
    if (mEndNodeNum >= mNodes.count()) {
        // Allocate a node only when needed, otherwise re-use one.
        mNodes.append (new DLXNode);
    }

    return mNodes.at (mEndNodeNum);
}

void DLXSolver::initNode (DLXNode * node)
{
    node->left = node->right = node;
    node->above = node->below = node;
    node->columnHeader = node;
    node->value = 0;
}

void DLXSolver::addAtRight (DLXNode * node, DLXNode * start)
{
    node->right        = start;
    node->left         = start->left;
    start->left        = node;
    node->left->right  = node;
}

void DLXSolver::addBelow (DLXNode * node, DLXNode * start)
{
    node->below        = start;
    node->above        = start->above;
    start->above       = node;
    node->above->below = node;
}

void DLXSolver::deleteAll()
{
    qDebug() << "DLXSolver::deleteAll() CALLED";
    qDeleteAll (mNodes);	// Deallocate the nodes pointed to.
    mNodes.clear();
    qDeleteAll (mColumns);	// Deallocate the column headers pointed to.
    mColumns.clear();
    mRows.clear();		// Secondary pointers: no nodes to deallocate.
}

#include "dlxsolver.moc"
