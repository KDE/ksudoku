/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#ifndef _PUZZLEPRINTER_H_
#define _PUZZLEPRINTER_H_

#include <QPen>

namespace ksudoku {
class Game;
class Puzzle;
}

class SKGraph;
class QWidget;

class PuzzlePrinter : public QObject
{
    Q_OBJECT

public:
    /**
     * Default Constructor
     */
	PuzzlePrinter (QWidget * parent);

    /**
     * Default Destructor
     */
	virtual ~PuzzlePrinter();
	
	void print (const ksudoku::Game & game);
	void endPrint();

private:
	enum Edge {Left = 0, Right, Above, Below, Detached};

	bool setupOutputDevices (int leastCellsToFit, int puzzleWidth);

	void drawBlocks (const ksudoku::Puzzle * puzzle,
			 const SKGraph * graph);
	void drawCages  (const ksudoku::Puzzle * puzzle,
			 const SKGraph * graph, bool killerStyle);
	void drawKillerSudokuCages (const SKGraph* graph,
                                    const QVector<int> & edges);
	void markEdges  (const QVector<int> & cells,
			 const ksudoku::Puzzle * puzzle, const SKGraph * graph,
			 QVector<int> & edges);
	void drawCell   (int posX, int posY, int edge);
	void drawValues (const ksudoku::Game & game, const SKGraph * graph);
	void drawCageLabel (const SKGraph* graph, int n, bool killerStyle);

	QWidget  * m_parent;
	QPrinter * m_printer;
	QPainter * m_p;
	int m_quadrant;
	int m_across;
	int m_down;
	bool m_printMulti;
	int m_sCell;
	int m_topX;
	int m_topY;
	QPen m_heavy;
	QPen m_light;
	QPen m_dashes;
};

#endif // _PUZZLEPRINTER_H_
