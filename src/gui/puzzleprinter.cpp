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
	m_quadrant(0)
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
    const bool printMulti = Settings::printMulti();
    const int  across  = 2;
    const int  down    = 2;
    const QString labels = (graph->base() <= 3) ? "123456789" :
                                                  "ABCDEFGHIJKLMNOPQRSTUVWXY";
    enum Edge {Left = 0, Right, Above, Below, Detached};
    const int All = (1 << Left) + (1 << Right) + (1 << Above) + (1 << Below);

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
            return;
        }
    }
    if (m_p == 0) {
        m_p = new QPainter (m_printer);	// Start a new print job.
    }

    QVector<int> edges (graph->size(), 0);
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
        for (int k = 0; k < order; k++) {
            int cell = clique.at (k);
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
                for (int cl = 0; cl < order; cl++) {
                    if (clique.at (cl) == nb[neighbour]) {
                        edge = edge - (1 << neighbour);
                    }
                }
            }
            edge = (edge == All) ? (1 << Detached) : edge;
            edges [cell] |= edge;	// Merge the edges found for this cell.
        }
    }

    // Calculate the printed dimensions of the puzzle.
    m_printer->setFullPage (false);		// Allow minimal margins.
    int pixels  = qMin (m_printer->width(), m_printer->height());
    int size    = pixels - (pixels / 20);	// Allow about 2.5% each side.
    int divs    = (graph->sizeX() > 20) ? graph->sizeX() : 20;
    int sCell   = size / divs;			// Size of each cell.
    size        = graph->sizeX() * sCell;	// Size of the whole puzzle.

    // Check if we require more than one puzzle per page and if they would fit.
    bool manyUp = printMulti && (pixels > (across * size));
    int margin1 = manyUp ? (pixels - across * size) / (across + 1)	// > 1.
                         : (pixels - size) / 2;				// = 1.
    pixels      = qMax (m_printer->width(), m_printer->height());
    int margin2 = manyUp ? (pixels - down * size) / (down + 1)		// > 1.
                         : (pixels - size) / 2;				// = 1.

    // Check for landscape vs. portrait mode and set the margins accordingly.
    int topX    = (m_printer->width() < m_printer->height())? margin1 : margin2;
    int topY    = (m_printer->width() < m_printer->height())? margin2 : margin1;

    if ((m_quadrant > 0) && (! manyUp)) {
        m_printer->newPage();			// Page has previous output.
        m_quadrant = 0;
    }
    topX = manyUp ? topX + (m_quadrant % across) * (topX + size) : topX;
    topY = manyUp ? topY + (m_quadrant / across) * (topY + size) : topY;
    m_quadrant = manyUp ? (m_quadrant + 1) : (across * down);

    // Set up parameters for the heavy and light line-drawing.
    int thin    = sCell / 40;	// Allow 2.5%.
    int thick   = (thin > 0) ? 3 * thin : 3;

    QPen light (QColor("#888888"));
    QPen heavy (QColor(QString("black")));
    light.setWidth (thin);
    heavy.setWidth (thick);
    heavy.setCapStyle (Qt::RoundCap);

    // Set font size 60% height of cell. Do not draw gray lines on top of black.
    QFont    f = m_p->font();
    f.setPixelSize ((sCell * 6) / 10);
    m_p->setFont (f);
    m_p->setCompositionMode (QPainter::CompositionMode_Darken);

    // Draw each cell in the puzzle.
    for (int n = 0; n < graph->size(); n++) {
        if (puzzle->value (n) < 0) {
            continue;				// Do not draw unused cells.
        }
        int x = topX + sCell * graph->cellPosX (n);
        int y = topY + sCell * graph->cellPosY (n);
        QRect rect (x, y, sCell, sCell);
        int edge = edges.at (n);
        if (edge & (1 << Detached)) {		// Shade a cell, as in XSudoku.
            m_p->fillRect (rect, QColor ("#DDDDDD"));
        }
        m_p->setPen (light);			// First draw every cell light.
        m_p->drawRect (rect);
        m_p->setPen (heavy);			// Draw block boundaries heavy.
        if (edge & (1<<Left))  m_p->drawLine (x, y, x, y + sCell);
        if (edge & (1<<Right)) m_p->drawLine (x+sCell, y, x+sCell, y+sCell);
        if (edge & (1<<Above)) m_p->drawLine (x, y, x+sCell, y);
        if (edge & (1<<Below)) m_p->drawLine (x, y+sCell, x+sCell, y+sCell);

        // Draw original puzzle values heavy: filled-in/solution values light.
        int v = game.value (n) - 1;
        if (v >= 0) {				// Skip empty cells.
            m_p->setPen ((puzzle->value (n) > 0) ? heavy : light);
            m_p->drawText (rect, Qt::AlignCenter, labels.mid (v, 1));
        }
    }
    if ((! manyUp) || (m_quadrant >= (across * down))) {
        endPrint();				// Print immediately.
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

#include "puzzleprinter.moc"
