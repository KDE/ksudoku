/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include "ksview.h"

#include "ksudokugame.h"

#include <qpixmap.h>
#include <qpainter.h>

#include "ksview.moc"

#include "sudoku_solver.h"
#include "puzzle.h"

#include <QtDebug>

namespace ksudoku{

KsView::KsView(const Game& game, QObject* parent)
	: QObject(parent), m_game(game), m_viewWidget(0)
{
	m_symbolTable = 0;
	m_currentValue = 1;
	m_currentCell = 0;
}

KsView::~KsView()
{
}

void KsView::draw(QPainter& /*p*/, int /*height*/, int /*width*/) const
{ ///@TODO improve performance (low priority)
//	//get user view
//	QPixmap const qp(const_cast< KsView* >(this)->renderPixmap(width, height, FALSE));
//	//copy to QPainter
//	p.drawPixmap(0,0,qp,-1,-1);
}

void KsView::setCursor(int cell) {
	m_currentCell = cell;
	emit cursorMoved(cell);
}

void KsView::selectValue(int value) {
	m_currentValue = value;
	emit valueSelected(value);
}

void KsView::enterValue(int value) {
	if(!m_game.given(m_currentCell)) {
		m_game.setValue(m_currentCell, value);
	}
}

void KsView::markValue(int value) {
	if(!m_game.given(m_currentCell)) {
		m_game.flipMarker(m_currentCell, value);
	}
}

void KsView::moveUp() {
	Graph* g = m_game.puzzle()->solver()->g;
	int x = g->cellPosX(m_currentCell);
	int y = g->cellPosY(m_currentCell);
	int z = g->cellPosZ(m_currentCell);
	
	if(--y < 0) y = g->sizeY()-1;
	setCursor(m_game.index(x,y,z));
}

void KsView::moveDown() {
	Graph* g = m_game.puzzle()->solver()->g;
	int x = g->cellPosX(m_currentCell);
	int y = g->cellPosY(m_currentCell);
	int z = g->cellPosZ(m_currentCell);
	
	if(++y >= g->sizeY()) y = 0;
	setCursor(m_game.index(x,y,z));
}

void KsView::moveLeft() {
	Graph* g = m_game.puzzle()->solver()->g;
	int x = g->cellPosX(m_currentCell);
	int y = g->cellPosY(m_currentCell);
	int z = g->cellPosZ(m_currentCell);
	
	if(--x < 0) x = g->sizeX()-1;
	setCursor(m_game.index(x,y,z));
}

void KsView::moveRight() {
	Graph* g = m_game.puzzle()->solver()->g;
	int x = g->cellPosX(m_currentCell);
	int y = g->cellPosY(m_currentCell);
	int z = g->cellPosZ(m_currentCell);
	
	if(++x >= g->sizeX()) x = 0;
	setCursor(m_game.index(x,y,z));
}

SymbolTable* KsView::symbolTable() const {
	return m_symbolTable;
}

void KsView::setSymbolTable(SymbolTable* table) {
	m_symbolTable = table;
	emit symbolsChanged(table);
}

void KsView::setWidget(QWidget* widget) {
	m_viewWidget = widget;
	emit flagsChanged(m_flags);
	emit symbolsChanged(m_symbolTable);
}

}
