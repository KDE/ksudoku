/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2012      Ian Wadham <iandw.au@gmail.com>                   *
 *   Copyright 2015      Ian Wadham <iandw.au@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "globals.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QLine>

#include <KLocale>

#include "puzzle.h"
#include "skgraph.h"
#include "ksudokugame.h"
#include "puzzleprinter.h"

#include "settings.h"

#include <KMessageBox>

PuzzlePrinter::PuzzlePrinter(QWidget * parent)
	:
	QObject(parent),
	m_parent(parent),
	m_printer(0),
	m_p(0),
	m_quadrant(0),
	m_across(2),
	m_down(2)
{
}

PuzzlePrinter::~PuzzlePrinter()
{
}

void PuzzlePrinter::print (const ksudoku::Game & game)
{
    const ksudoku::Puzzle * puzzle = game.puzzle();
    const SKGraph *         graph  = puzzle->graph();
    if (graph->sizeZ() > 1) {
        KMessageBox::information (m_parent,
            i18n("Sorry, cannot print three-dimensional puzzles."));
        return;
    }
    const int leastCellsToFit = 20;	// Avoids huge cells in small puzzles.

    // Set up a QPrinter and a QPainter and allocate space on the page.
    bool pageFilled = setupOutputDevices (leastCellsToFit, graph->sizeX());
    if (! m_printer) {
	return;				// The user did not select a printer.
    }

    // Draw the puzzle grid and its contents.
    bool hasBlocks   = (graph->specificType() != Mathdoku);
    bool hasCages    = ((graph->specificType() == Mathdoku) ||
	                (graph->specificType() == KillerSudoku));
    bool killerStyle = (graph->specificType() == KillerSudoku);

    if (hasBlocks) {			// Only Mathdoku has no blocks.
	drawBlocks (puzzle, graph);
    }
    if (hasCages) {			// Mathdoku and KillerSudoku have cages.
	drawCages (puzzle, graph, killerStyle);	// KillerSudoku has blocks too.
    }
    drawValues (game, graph);		// Print starting and filled-in values.

    if (pageFilled) {
        endPrint();			// Print immediately.
    }
    else {
        KMessageBox::information (m_parent,
            i18n ("The KSudoku setting for printing several puzzles per page "
                  "is currently selected.\n\n"
                  "Your puzzle will be printed when no more will fit on the "
                  "page or when KSudoku terminates."));
    }
}

void PuzzlePrinter::endPrint()
{
    if (m_p != 0) {
        // The current print output goes to the printer when the painter ends.
        m_p->end();
        delete m_p;
        m_p = 0;
        m_quadrant = 0;
        KMessageBox::information (m_parent,
            i18n ("KSudoku has sent output to your printer."));
    }
}

bool PuzzlePrinter::setupOutputDevices (int leastCellsToFit, int puzzleWidth)
{
    // The printer and painter objects can persist between print requests, so
    // (if required) we can print several puzzles per page and defer printing
    // until the page is full or KSudoku terminates and the painter ends itself.
    // NOTE: Must create painter before using functions like m_printer->width().
    if (m_printer == 0) {
        m_printer = new QPrinter (QPrinter::HighResolution);
        QPrintDialog * dialog = new QPrintDialog(m_printer, m_parent);
        dialog->setWindowTitle(i18n("Print Sudoku Puzzle"));
        if (dialog->exec() != QDialog::Accepted) {
            delete m_printer;
            m_printer = 0;
            return false;
        }
    }
    if (m_p == 0) {
        m_p = new QPainter (m_printer);	// Start a new print job.
    }
    m_printMulti = Settings::printMulti();

    // Calculate the printed dimensions of the puzzle.
    m_printer->setFullPage (false);		// Allow minimal margins.
    int pixels  = qMin (m_printer->width(), m_printer->height());
    int space   = pixels - (pixels / 20);	// Allow about 2.5% each side.
    int pCells  = qMax (leastCellsToFit, puzzleWidth);	// Cells to allocate.
    m_sCell     = space / pCells;		// Size of each cell.
    int size    = puzzleWidth * m_sCell;	// Size of the whole puzzle.

    // Check if we require more than one puzzle per page and if they would fit.
    bool manyUp = m_printMulti && (pixels > (m_across * size));
    int margin1 = manyUp ? (pixels - m_across * size) / (m_across + 1)	// > 1.
                         : (pixels - size) / 2;				// = 1.
    pixels      = qMax (m_printer->width(), m_printer->height());
    int margin2 = manyUp ? (pixels - m_down * size) / (m_down + 1)	// > 1.
                         : (pixels - size) / 2;				// = 1.

    // Check for landscape vs. portrait mode and set the margins accordingly.
    m_topX = (m_printer->width() < m_printer->height())? margin1 : margin2;
    m_topY = (m_printer->width() < m_printer->height())? margin2 : margin1;

    // If new puzzle will not fit a quadrant and page is not empty, flush page.
    if ((m_quadrant > 0) && (! manyUp)) {
        m_printer->newPage();
        m_quadrant = 0;
    }
    m_topX = manyUp ? m_topX + (m_quadrant%m_across) * (m_topX + size) : m_topX;
    m_topY = manyUp ? m_topY + (m_quadrant/m_across) * (m_topY + size) : m_topY;
    m_quadrant = manyUp ? (m_quadrant + 1) : (m_across * m_down);

    // Set up pen-parameters for the heavy and light line-drawing.
    int thin    = m_sCell / 40;	// Allow 2.5%.
    int thick   = (thin > 0) ? 2 * thin : 2;

    m_light.setColor (QColor("#888888"));
    m_light.setWidth (thin);
    m_heavy.setColor (QColor(QString("black")));
    m_heavy.setWidth (thick);
    m_heavy.setCapStyle (Qt::RoundCap);
    m_dashes.setColor (QColor(QString("black")));
    m_dashes.setWidth (thin);
    m_dashes.setStyle (Qt::DashLine);

    // Return true if the page will be filled up after drawing the puzzle.
    return ((! manyUp) || (m_quadrant >= (m_across * m_down)));
}

void PuzzlePrinter::drawBlocks (const ksudoku::Puzzle * puzzle,
				const SKGraph * graph)
{
    QVector<int> edges (graph->size(), 0);	// One bitmap per cell.
    int order = graph->order();

    for (int n = 0; n < graph->cliqueCount(); n++) {
        // Find out which groups are blocks of cells, not rows or columns.
        QVector<int> clique = graph->clique (n);
        int x = graph->cellPosX (clique.at (0));
        int y = graph->cellPosY (clique.at (0));
        bool isRow = true;
        bool isCol = true;
        for (int k = 1; k < order; k++) {
            if (graph->cellPosX (clique.at (k)) != x) isRow = false;
            if (graph->cellPosY (clique.at (k)) != y) isCol = false;
        }
        if (isRow || isCol) continue;	// Skip rows and columns.

        // Mark the outside edges of each block.
	markEdges (clique, puzzle, graph, edges);
    }

    // Draw each cell in the puzzle.
    for (int n = 0; n < graph->size(); n++) {
        if (puzzle->value (n) < 0) {
            continue;				// Do not draw unused cells.
	}
	drawCell (graph->cellPosX (n), graph->cellPosY (n), edges.at (n));
    }
}

void PuzzlePrinter::drawCages (const ksudoku::Puzzle * puzzle,
			       const SKGraph * graph, bool killerStyle)
{
    QVector<int> edges (graph->size(), 0);	// One bitmap per cell.
    for (int n = 0; n < graph->cageCount(); n++) {
        // Mark the outside edges of each cage.
	markEdges (graph->cage (n), puzzle, graph, edges);
    }
    if (killerStyle) {
	drawKillerSudokuCages (graph, edges);
    }
    else {
	// Draw each cell in the puzzle.
	for (int n = 0; n < graph->size(); n++) {
	    if (puzzle->value (n) < 0) {
		continue;			// Do not draw unused cells.
	    }
	    drawCell (graph->cellPosX (n), graph->cellPosY (n), edges.at (n));
	}
    }
    for (int n = 0; n < graph->cageCount(); n++) {
	drawCageLabel (graph, n, killerStyle);
    }
}

void PuzzlePrinter::markEdges (const QVector<int> & cells,
			       const ksudoku::Puzzle * puzzle,
			       const SKGraph * graph, QVector<int> & edges)
{
    const int All = (1 << Left) + (1 << Right) + (1 << Above) + (1 << Below);

    int nCells = cells.count();
    for (int k = 0; k < nCells; k++) {
	int cell = cells.at (k);
	int x = graph->cellPosX (cell);
	int y = graph->cellPosY (cell);
	int nb[4] = {-1, -1, -1, -1};
	int lim = graph->sizeX() - 1;

	// Start with all edges: remove them as neighbours are found.
	int edge = All;
	nb[Left]  = (x > 0)   ? graph->cellIndex (x-1, y) : -1;
	nb[Right] = (x < lim) ? graph->cellIndex (x+1, y) : -1;
	nb[Above] = (y > 0)   ? graph->cellIndex (x, y-1) : -1;
	nb[Below] = (y < lim) ? graph->cellIndex (x, y+1) : -1;
	for (int neighbour = 0; neighbour < 4; neighbour++) {
	    if ((nb[neighbour] < 0) || (puzzle->value(nb[neighbour]) < 0)) {
		continue;		// No neighbour on this side.
	    }
	    for (int cl = 0; cl < nCells; cl++) {
		if (cells.at (cl) == nb[neighbour]) {
		    edge = edge - (1 << neighbour);
		}
	    }
	}
	// Check for size-1 cages or detached cells as in XSudoku diagonals.
	if ((edge == All) && (graph->specificType() != Mathdoku) &&
	    (graph->specificType() != KillerSudoku)) {
	    edge = (1 << Detached);
	}
	edges [cell] |= edge;	// Merge the edges found for this cell.
    }
}

void PuzzlePrinter::drawCell (int posX, int posY, int edge)
{
    int x = m_topX + m_sCell * posX;
    int y = m_topY + m_sCell * posY;
    QRect rect (x, y, m_sCell, m_sCell);
    if (edge & (1 << Detached)) {		// Shade a cell, as in XSudoku.
	m_p->fillRect (rect, QColor ("#DDDDDD"));
    }
    m_p->setPen (m_light);			// First draw every cell light.
    m_p->drawRect (rect);
    m_p->setPen (m_heavy);			// Draw block boundaries heavy.
    if (edge & (1<<Left))  m_p->drawLine (x, y, x, y + m_sCell);
    if (edge & (1<<Right)) m_p->drawLine (x+m_sCell, y, x+m_sCell, y+m_sCell);
    if (edge & (1<<Above)) m_p->drawLine (x, y, x+m_sCell, y);
    if (edge & (1<<Below)) m_p->drawLine (x, y+m_sCell, x+m_sCell, y+m_sCell);
}

void PuzzlePrinter::drawValues (const ksudoku::Game& game, const SKGraph* graph)
{
    const QString labels = (graph->base() <= 3) ? "123456789" :
                                                  "ABCDEFGHIJKLMNOPQRSTUVWXY";
    // Set font size 60% height of cell.
    QFont f = m_p->font();
    f.setPixelSize ((m_sCell * 6) / 10);
    m_p->setFont (f);

    for (int n = 0; n < graph->size(); n++) {
        int v = game.value (n) - 1;
	if (v < 0) {
	    continue;                           // Skip empty or unused cells.
	}

        // Draw original puzzle values heavy: filled-in/solution values light.
	int x = m_topX + m_sCell * graph->cellPosX (n);
	int y = m_topY + m_sCell * graph->cellPosY (n);
	QRect rect (x, y, m_sCell, m_sCell);
	m_p->setPen (game.given (n) ? m_heavy : m_light);
	m_p->drawText (rect, Qt::AlignCenter, labels.mid (v, 1));
    }
}

void PuzzlePrinter::drawKillerSudokuCages (const SKGraph* graph,
                                           const QVector<int> & edges)
{
    // Killer Sudokus have cages AND groups: so the cages are drawn differently.
    // We keep the outer wall of the cage on our left and draw a dashed line
    // just inside that boundary.  This reduces ugly criss-crossing of lines.
    //
    // Directions and related arrays are all in clockwise order.
    enum  Direction {East, South, West, North, nDirections};
    const Direction rightTurn [nDirections] = {South, West, North, East};
    const Direction leftTurn [nDirections]  = {North, East, South, West};
    const int       wallOnLeft [nDirections] =
			    {1 << Above, 1 << Right, 1 << Below, 1 << Left};
    const int       wallAhead [nDirections] =
			    {1 << Right, 1 << Below, 1 << Left, 1 << Above};

    const int       deltaX  [nDirections] = {+1, 0, -1, 0};
    const int       deltaY  [nDirections] = {0, +1, 0, -1};

    int   cellInc [nDirections] = {graph->order(), +1, -graph->order(), -1};
    int   offset    = (m_sCell + 6) / 12;
    int   longSide  = m_sCell;
    int   shortSide = m_sCell - offset - offset;

    m_p->setPen (m_dashes);

    for (int n = 0; n < graph->cageCount(); n++) {
	int topLeft = graph->cageTopLeft(n);
	int cell    = topLeft;
	int edge    = edges.at (cell);
	int startX  = m_topX + m_sCell * graph->cellPosX (cell) + offset;
	int startY  = m_topY + m_sCell * graph->cellPosY (cell) + offset;
	int dx      = 0;
	int dy      = 0;
	QLine line (startX, startY, startX, startY);
	Direction direction = East;

	// Keep drawing until we get back to the starting cell and direction.
	do {
	    // If there is a wall on the left, follow it.
	    if (edge & wallOnLeft [direction]) {
		if (edge & wallAhead [direction]) {
		    // Go to wall (shortSide), draw line, turn right, new line.
		    dx = deltaX [direction] * shortSide;
		    dy = deltaY [direction] * shortSide;
		    line.setLine (line.x1(), line.y1(),
				  line.x2() + dx, line.y2() + dy);
		    m_p->drawLine (line);
		    direction = rightTurn [direction];
		    line.setLine (line.x2(), line.y2(), line.x2(), line.y2());
		}
		else {
		    // Extend to start of next cell (longSide).
		    dx = deltaX [direction] * longSide;
		    dy = deltaY [direction] * longSide;
		    line.setLine (line.x1(), line.y1(),
				  line.x2() + dx, line.y2() + dy);
		    cell = cell + cellInc [direction];
		    edge = edges.at (cell);
		}
	    }
	    // Else, if there is no wall on the left ...
	    else {
		// Draw line, turn left, new line, go to start of next cell.
		m_p->drawLine (line);
		direction = leftTurn [direction];
		dx = deltaX [direction] * (longSide - shortSide);
		dy = deltaY [direction] * (longSide - shortSide);
		line.setLine (line.x2(), line.y2(),
			      line.x2() + dx, line.y2() + dy);
		cell = cell + cellInc [direction];
		edge = edges.at (cell);

	    }
	} while (! ((cell == topLeft) && (direction == East)));
    }	// Draw next cage.
}

void PuzzlePrinter::drawCageLabel (const SKGraph* graph, int n,
				   bool killerStyle)
{
    if (graph->cage (n).size() < 2) {
	return;
    }

    int topLeft = graph->cageTopLeft (n);
    int cellX = m_topX + m_sCell * graph->cellPosX (topLeft);
    int cellY = m_topY + m_sCell * graph->cellPosY (topLeft);

    QString cLabel = QString::number (graph->cageValue (n));
    if (! killerStyle) {	// No operator is shown in KillerSudoku.
	cLabel = cLabel + QString(" /-x+").mid(graph->cageOperator (n), 1);
    }

    QFont f = m_p->font();
    f.setPixelSize ((m_sCell + 3)/4);
    f.setBold (true);
    m_p->setFont(f);
    QFontMetrics fm(f);
    int w = fm.width(cLabel);
    int a = fm.ascent();
    int m = (fm.width(QChar('1'))+1)/3;	// Left margin = 1/3 width of '1'.

    if (killerStyle) {
	// Cover part of the dashed line, to make a background for the text.
	m_p->fillRect(cellX + m, cellY + m, w + w/10, a /* h */, Qt::white);
    }
    // Note: Origin of text is on baseline to left of first character.
    m_p->drawText (cellX + m, cellY + a, cLabel);
}

#include "puzzleprinter.moc"
