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

#include "ksudokuview.h"

#include "sudoku_solver.h"

#include <qpainter.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <kmessagebox.h>
#include <klocale.h>
		
#include <math.h>
#include "puzzle.h"


namespace ksudoku {

ksudokuView::ksudokuView(QWidget *parent, const Game& game, bool customd)
	: QWidget(parent)
	, KsView()
{
	isWaitingForNumber = -1;
	highlighted = -1;
	
	custom = customd;
	m_color0 = 0;
// 	m_buttons.setAutoDelete(true);
	
	m_guidedMode = true;
	current_selected_number = 0;

	setGame(game);
}

// void ksudokuView::setup(const ksudoku::Game& game)
// {
// //	std::srand( time(0) );//srandom((unsigned)time( NULL ) );
// //	puzzle_mark_wrong=true;
// 	
// }

ksudokuView::~ksudokuView()
{
	for(int i = 0; i < m_buttons.size(); ++i) {
		delete m_buttons[i];
		m_buttons[i] = 0;
	}
}

QString ksudokuView::status() const
{
	QString m;

	int secs = QTime(0,0).secsTo(m_game.time());
	if(secs % 36 < 12)
		m = i18n("Selected item %1, Time elapsed %2. Press SHIFT to highlight.",
		         m_game.value2Char((current_selected_number > 0) ? current_selected_number : 0),
		         m_game.time().toString("hh:mm:ss"));
	else if(secs % 36 < 24)
		m = i18n("Selected item %1, Time elapsed %2. Use RMB to pencil-mark(superscript).",
		         m_game.value2Char((current_selected_number > 0) ? current_selected_number : 0),
		         m_game.time().toString("hh:mm:ss"));
	else
		m = i18n("Selected item %1, Time elapsed %2. Type in a cell to replace that number in it.",
		         m_game.value2Char((current_selected_number > 0) ? current_selected_number : 0),
		         m_game.time().toString("hh::mm::ss"));

	return m;
}

void ksudokuView::draw(QPainter& p, int height, int width) const
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


void ksudokuView::btn_enter(int x, int y)
{
	//for wrapping:
	int max_x = ((GraphCustom*) m_game.puzzle()->solver()->g)->sizeX() - 1;
	int max_y = ((GraphCustom*) m_game.puzzle()->solver()->g)->sizeY() - 1;
	if(x > max_x+1) x = max_x;  //uint => negative == max uint
	if(y > max_y+1) y = max_y;
	if(x > max_x) x = 0;
	if(y > max_y) y = 0;

	m_buttons[m_game.index(x,y)]->setFocus();

	isWaitingForNumber = -1;

	for(int i = 0; i < m_game.size(); ++i) {
		m_buttons[i]->setHighlight(HighlightBaseMask, HighlightNone);
	}
	

	if(custom==0)
	{

		for(int i = 0; i < m_game.order(); ++i)
		{
	// 		uint base = m_game.userPuzzle()->base;
			int base = static_cast<int>(sqrt(m_game.puzzle()->order()));
			m_buttons[m_game.index(i, y)]->setHighlight(HighlightRow);
			m_buttons[m_game.index(x, i)]->setHighlight(HighlightColumn);
			int sx = (x/base) * base;
			int sy = (y/base) * base;
			m_buttons[m_game.index(i/base+sx, sy+(i%base))]->setHighlight(HighlightClique);
		}
	}
	else
	{
		/*int colorA = m_color0;
		m_color0 = (m_color0+1)%2;
		int colorB = m_color0;
		GraphCustom* g = ((GraphCustom*) m_game.puzzle()->solver()->g);
		int index = m_game.index(y,x);
		m_buttons[index]->setHighlighted(colorA+1,true);
		for(int i=0; i<g->optimized_d[index]; i++)
		{
			m_buttons[g->optimized[index][i]]->setHighlighted(colorB+1,true);
		}*/
		GraphCustom* g = ((GraphCustom*) m_game.puzzle()->solver()->g);
		int index = g->cellIndex(x,y);
		//printf("(%d %d) %d\n", y, x, index);
		int count = 0;

		for(int i=0; i<g->cliques.size(); i++)
		{
			for(int j=0; j<g->cliques[i].size(); j++)
			{
				if(g->cliques[i][j]==index)
				{
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
					for(int k=0; k<g->cliques[i].size(); k++)
					{
						m_buttons[g->cliques[i][k]]->setHighlight(mask);
					}
					count = (count+1)%3;
					break;
				}
			}
		}
	}
	for(int i = 0; i < m_game.size(); ++i)
		m_buttons[i]->update();
}


void ksudokuView::btn_leave(int /*x*/, int /*y*/)
{}

void ksudokuView::setGame(const ksudoku::Game& game) {
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
			QSudokuButton* btn = new QSudokuButton(this, 0, 0);
				
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
			QSudokuButton* btn = new QSudokuButton((ksudokuView*) this, 0, 0);

			btn->setCustom(1);

			if(!notConnectedNode)
			{
				btn->setConnected(true);
				connect(btn, SIGNAL(clicked2(int, int)), this, SLOT(slotHello(int, int)));
				connect(btn, SIGNAL(rightclicked(int, int)), this, SLOT(slotRight(int, int)));
				connect(btn, SIGNAL(enter(int,int)), this, SLOT(btn_enter(int,int)));
				connect(btn, SIGNAL(leave(int,int)), this, SLOT(btn_leave(int,int)));
				connect(btn, SIGNAL(beginHighlight(int)), this, SLOT(beginHighlight(int)));
				connect(btn, SIGNAL(finishHighlight()), this, SLOT(finishHighlight()));
			}
			else
			{
				btn->setConnected(false);
			}
			m_buttons[i] = btn;
		}
		
	
	}
	for(int i = 0; i < m_buttons.size(); ++i) {
		m_buttons[i]->setY(g->cellPosY(i));
		m_buttons[i]->setX(g->cellPosX(i));
		m_buttons[i]->updateData();
		m_buttons[i]->resize();
		m_buttons[i]->show();
	}

	connect(m_game.interface(), SIGNAL(cellChange(int)), this, SLOT(onCellChange(int)));
	connect(m_game.interface(), SIGNAL(fullChange()), this, SLOT(onFullChange()));
	
	connect(m_game.interface(), SIGNAL(completed(bool,const QTime&,bool)), parent(), SLOT(onCompleted(bool,const QTime&,bool)));
	connect(m_game.interface(), SIGNAL(modified(bool)), parent(), SLOT(onModified(bool)));

	//printf("DONE\n");
}

void ksudokuView::beginHighlight(int val)
{
	if( ! m_game.hasSolver()) return;

	highlighted = val;
	if(val == 0) highlighted = current_selected_number;
	if(highlighted == -1) return;
	
	QBitArray highlightedBtns = m_game.highlightValueConnections(highlighted, true);
	if(highlightedBtns.size() < m_game.size())
		return;
	
	for(int i = 0; i < m_game.size(); ++i) {
		m_buttons[i]->setHighlight(HighlightHelpMask, highlightedBtns[i] ? HighlightShowHelp : HighlightHelpMask);
		m_buttons[i]->update();
	}
}

void ksudokuView::finishHighlight()
{
	highlighted = -1;

	for(int i = 0; i < m_game.size(); ++i)
	{
		m_buttons[i]->setHighlight(HighlightHelpMask, HighlightNone);
		m_buttons[i]->update();
	}
}
	
void ksudokuView::resizeEvent(QResizeEvent * /*event*/ )
{
	for(int i = 0; i < m_buttons.size(); ++i)
		m_buttons[i]->resize();
}
	
void ksudokuView::slotHello(int x, int y)
{
	if(m_game.given(x,y))
	{
		current_selected_number = m_game.value(x,y);
		emit changedSelectedNum();
	}
	else
	{
		m_game.setValue(current_selected_number,x,y);
	}
}

void  ksudokuView::slotRight(int x, int y)
{
	if(!m_game.given(x,y))
	{
		if(mouseOnlySuperscript == 1)
		{
			m_game.flipMarker(current_selected_number, x, y);
		}
		else
		{
			isWaitingForNumber = m_game.index(x,y);
			m_buttons[m_game.index(x,y)]->update();
		}
	}
}

void ksudokuView::onCellChange(int index) {
	if(m_buttons[index])
		m_buttons[index]->updateData();
}

void ksudokuView::onFullChange() {
	for(int i = 0; i < m_buttons.count(); i++)
		m_buttons[i]->updateData();
}

}

#include "ksudokuview.moc"
