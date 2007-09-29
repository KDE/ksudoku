/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include "sudokuview.h"

#include "sudoku_solver.h"

#include <qpainter.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <kmessagebox.h>
#include <klocale.h>
		
#include <math.h>
#include "puzzle.h"

#include "symbols.h"

#include "valuelistwidget.h"

namespace ksudoku {

SudokuView::SudokuView(QWidget *parent, const Game& game, bool customd)
	: QWidget(parent)
{
	m_symbolTable = 0;
	highlighted = -1;
	
	custom = customd;
	
	current_selected_number = 1;

	setGame(game);
}

// void SudokuView::setup(const ksudoku::Game& game)
// {
// //	std::srand( time(0) );//srandom((unsigned)time( NULL ) );
// //	puzzle_mark_wrong=true;
// 	
// }

SudokuView::~SudokuView()
{
	for(int i = 0; i < m_buttons.size(); ++i) {
		delete m_buttons[i];
		m_buttons[i] = 0;
	}
}

QString SudokuView::status() const
{
	QString m;
	
	if(!m_symbolTable) return m;

	int secs = QTime(0,0).secsTo(m_game.time());
	if(secs % 36 < 12)
		m = i18n("Selected item %1, Time elapsed %2. Press SHIFT to highlight.",
		         m_symbolTable->symbolForValue(current_selected_number),
		         m_game.time().toString("hh:mm:ss"));
	else if(secs % 36 < 24)
		m = i18n("Selected item %1, Time elapsed %2. Use RMB to pencil-mark(superscript).",
		         m_symbolTable->symbolForValue(current_selected_number),
		         m_game.time().toString("hh:mm:ss"));
	else
		m = i18n("Selected item %1, Time elapsed %2. Type in a cell to replace that number in it.",
		         m_symbolTable->symbolForValue(current_selected_number),
		         m_game.time().toString("hh::mm::ss"));

	return m;
}

QChar SudokuView::symbol(int value) const {
	if(m_symbolTable == 0)
		return '\0';
	return m_symbolTable->symbolForValue(value);
}


void SudokuView::draw(QPainter& p, int height, int width) const
{
	if(m_buttons.size() == 0)
		return;

	int w = m_buttons[0]->width ();
	int h = m_buttons[0]->height();
	int o = m_game.order();

	//must fit on height width
	float scaleH = static_cast<float>(height) / (h*o);
	float scaleW = static_cast<float>(width)  / (w*o);
	p.scale(scaleW,scaleH);

	for(int i = 0; i < m_buttons.size(); ++i){
		if(!m_buttons[i]) continue;
		int x,y;
		if(custom==0)
		{
			x = w * (i/o);
			y = h * (i%o);
		}
		else
		{
			x = w* m_buttons[i]->getX();
			y = w* m_buttons[i]->getY();
		}
		p.translate(x,y);
		m_buttons[i]->drawExt(p);
		p.translate(-x,-y);  //avoid resetMatrix()
	}
}


void SudokuView::btn_enter(int x, int y) {
	emit cellHovered(m_game.index(x,y));
}

void SudokuView::setCursor(int cell) {
	if(!m_buttons[cell]) return;
	
	m_buttons[cell]->setFocus();
	m_currentCell = cell;
	
	Graph* g = m_game.puzzle()->solver()->g;
	int x = g->cellPosX(cell);
	int y = g->cellPosY(cell);

	if(m_highlightUpdate.size() != m_game.size())
		m_highlightUpdate.resize(m_game.size());
	
	for(int i = 0; i < m_game.size(); ++i) {
		m_highlightUpdate[i] = HighlightNone;
	}
	
	if(!m_flags.testFlag(ShowHighlights)) {
	} else if(custom==0) {
		for(int i = 0; i < m_game.order(); ++i) {
			int base = static_cast<int>(sqrt(m_game.puzzle()->order()));
			m_highlightUpdate[m_game.index(i, y)] |= HighlightRow;
			m_highlightUpdate[m_game.index(x, i)] |= HighlightColumn;
			int sx = (x/base) * base;
			int sy = (y/base) * base;
			m_highlightUpdate[m_game.index(i/base+sx,i%base+sy)] |= HighlightClique;
		}
	} else {
		GraphCustom* gc = dynamic_cast<GraphCustom*>(g);
		int count = 0;

		for(uint i=0; i<gc->cliques.size(); i++)
		{
			for(uint j=0; j<gc->cliques[i].size(); j++)
			{
				if(gc->cliques[i][j] != cell) continue;
				
				uint mask = HighlightNone;
				switch(count) {
					case 0:
						mask = HighlightColumn;
						break;
					case 1:
						mask = HighlightRow;
						break;
					default:
						mask = HighlightClique;
						break;
				}
				for(uint k=0; k<gc->cliques[i].size(); k++)
				{
					m_highlightUpdate[gc->cliques[i][k]] |= mask;
				}
				count = (count+1)%3;
				break;
			}
		}
	}
	
	for(int i = 0; i < m_game.size(); ++i) {
		if(!m_buttons[i]) continue;
		m_buttons[i]->setHighlight(HighlightBaseMask, m_highlightUpdate[i]);
	}
}


void SudokuView::btn_leave(int /*x*/, int /*y*/)
{}

void SudokuView::setGame(const ksudoku::Game& game) {
	// TODO change this member so that its code can only be executed once
	// the best way might be removeing this member and move the oce somewhere else
	
// 	if(m_game.interface()) {
// 		disconnect(m_game.interface(), SIGNAL(cellChange(uint)), this, SLOT(onCellChange(uint)));
// 		disconnect(m_game.interface(), SIGNAL(fullChange()), this, SLOT(onFullChange()));
// 		
// 		disconnect(m_game.interface(), SIGNAL(completed(bool,const QTime&,bool)), parent(), SLOT(onCompleted(bool,const QTime&,bool)));
// 		disconnect(m_game.interface(), SIGNAL(modified(bool)), parent(), SLOT(onModified(bool)));
// 	}

	m_game = game;

	m_buttons.resize(m_game.size());
	
	Graph* g = m_game.puzzle()->solver()->g;
	if(custom==0)
	{
		for(int i = 0; i < m_game.size(); ++i) {
			int x = g->cellPosX(i);
			int y = g->cellPosY(i);
			QSudokuButton* btn = new QSudokuButton(this, x, y);
				
			connect(btn, SIGNAL(clicked2(int, int)), this, SLOT(slotHello(int, int)));
			connect(btn, SIGNAL(rightclicked(int, int)), this, SLOT(slotRight(int, int)));
			connect(btn, SIGNAL(enter(int,int)), this, SLOT(btn_enter(int,int)));
			connect(btn, SIGNAL(leave(int,int)), this, SLOT(btn_leave(int,int)));
				
			connect(btn, SIGNAL(beginHighlight(int)), this, SLOT(beginHighlight(int)));
			connect(btn, SIGNAL(finishHighlight()), this, SLOT(finishHighlight()));
			m_buttons[i] = btn;
		}
	}
	else
	{
		for(int i = 0; i < m_game.size(); ++i) {
			bool notConnectedNode = ((GraphCustom*) m_game.puzzle()->solver()->g)->optimized_d[i] == 0;
			
			if(notConnectedNode) {
				m_buttons[i] = 0;
				continue;
			}

			int x = g->cellPosX(i);
			int y = g->cellPosY(i);
			QSudokuButton* btn = new QSudokuButton(this, x, y);

			btn->setCustom(1);

			connect(btn, SIGNAL(clicked2(int, int)), this, SLOT(slotHello(int, int)));
			connect(btn, SIGNAL(rightclicked(int, int)), this, SLOT(slotRight(int, int)));
			connect(btn, SIGNAL(enter(int,int)), this, SLOT(btn_enter(int,int)));
			connect(btn, SIGNAL(leave(int,int)), this, SLOT(btn_leave(int,int)));
			connect(btn, SIGNAL(beginHighlight(int)), this, SLOT(beginHighlight(int)));
			connect(btn, SIGNAL(finishHighlight()), this, SLOT(finishHighlight()));
				
			m_buttons[i] = btn;
		}
		
	
	}
	for(int i = 0; i < m_buttons.size(); ++i) {
		if(!m_buttons[i]) continue;
		m_buttons[i]->updateData();
		m_buttons[i]->resize();
		m_buttons[i]->show();
	}

	connect(m_game.interface(), SIGNAL(cellChange(int)), this, SLOT(update(int)));
	connect(m_game.interface(), SIGNAL(fullChange()), this, SLOT(update()));
}

void SudokuView::selectValue(int value) {
	if(current_selected_number == value) return;
	current_selected_number = value;

	if(getHighlighted() != -1)
		beginHighlight(value);
	
	emit valueSelected(value);
}

void SudokuView::setSymbols(SymbolTable* table) {
	m_symbolTable = table;
	update();
}

void SudokuView::setFlags(ViewFlags flags) {
	m_flags = flags;
	update();
}

void SudokuView::beginHighlight(int val)
{
	if( ! m_game.hasSolver()) return;

	highlighted = val;
	if(val == 0) highlighted = current_selected_number;
	if(highlighted == -1) return;
	
	QBitArray highlightedBtns = m_game.highlightValueConnections(highlighted, true);
	if(highlightedBtns.size() < m_game.size())
		return;
	
	for(int i = 0; i < m_game.size(); ++i) {
		if(!m_buttons[i]) continue;
		m_buttons[i]->setHighlight(HighlightHelpMask, highlightedBtns[i] ? HighlightShowHelp : HighlightHelpMask);
		m_buttons[i]->update();
	}
}

void SudokuView::finishHighlight()
{
	highlighted = -1;

	for(int i = 0; i < m_game.size(); ++i)
	{
		if(!m_buttons[i]) continue;
		m_buttons[i]->setHighlight(HighlightHelpMask, HighlightNone);
		m_buttons[i]->update();
	}
}
	
void SudokuView::resizeEvent(QResizeEvent * /*event*/ )
{
	for(int i = 0; i < m_buttons.size(); ++i) {
		if(!m_buttons[i]) continue;
		m_buttons[i]->resize();
	}
}

void SudokuView::wheelEvent (QWheelEvent* e) {
	int order = m_game.order();
	int value = (current_selected_number - e->delta()/120) % order;
	if(value <= 0) value = order - value;
	
	selectValue(value);
}
	
void SudokuView::slotHello(int x, int y)
{
	if(m_game.given(x,y))
	{
		selectValue(m_game.value(x,y));
	}
	else
	{
		m_game.setValue(current_selected_number,x,y);
	}
}

void  SudokuView::slotRight(int x, int y)
{
	if(!m_game.given(x,y))
	{
		m_game.flipMarker(current_selected_number, x, y);
	}
}

void SudokuView::update(int cell) {
	if(cell < 0) {
		for(int i = 0; i < m_buttons.count(); i++) {
			if(!m_buttons[i]) continue;
			m_buttons[i]->updateData();
		}
		return;
	}
	if(m_buttons[cell])
		m_buttons[cell]->updateData();

	if(getHighlighted() != -1) beginHighlight(getHighlighted());
}

}

#include "sudokuview.moc"
