/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2012      Ian Wadham <iandw.au@gmail.com>                   *
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
#include "settings.h"



#include "puzzle.h"

#ifdef OPENGL_SUPPORT
#include "roxdokuview.h"
#endif

#include "view2d.h"

namespace ksudoku{

KsView::KsView(const Game& game, GameActions* gameActions, QObject* parent)
	: QObject(parent), m_game(game), m_gameActions(gameActions), m_viewWidget(0)
{
	m_symbolTable = 0;
	m_currentValue = 1;
	m_valueListWidget = 0;
}

KsView::~KsView()
{
	delete m_viewWidget;
}

void KsView::createView() {
	GameType type = m_game.puzzle()->gameType();
	switch(type) {
		case TypeSudoku: {
			setWidget(new View2D(0, m_game, m_gameActions));
			break;
		}
		case TypeCustom: {
#ifdef OPENGL_SUPPORT
			if(m_game.puzzle()->graph()->sizeZ() > 1) {
			    // TODO - IDW - Add a parent widget. Memory leak?
			    setWidget(new RoxdokuView(m_game, m_gameActions, 0));
			}
			else {
			    setWidget(new View2D(0, m_game, m_gameActions));
			}
#else
			setWidget(new View2D(0, m_game, m_gameActions));
#endif
			break;
		}
#ifdef OPENGL_SUPPORT
		case TypeRoxdoku: {
			// TODO - IDW - Add a parent widget. Memory leak?
			setWidget(new RoxdokuView(m_game, m_gameActions, 0));
			break;
		}
#endif
		default:
			// TODO this will not work
			break;
	}
	
	QObject* view = m_view->widget();
	connect(view, SIGNAL(valueSelected(int)), SLOT(selectValue(int)));
	
	connect(this, SIGNAL(valueSelected(int)), view, SLOT(selectValue(int)));

	connect(parent(), SIGNAL(settingsChanged()), view, SLOT(settingsChanged()));
	
	settingsChanged();
}

void KsView::selectValue(int value) {
	if(value == m_currentValue) return;
	m_currentValue = value;
	emit valueSelected(value);
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
	
	m_view = dynamic_cast<ViewInterface*>(widget);
}

void KsView::settingsChanged() {
	ViewFlags flags;
	if(Settings::showErrors()) flags |= ShowErrors;
	if(Settings::showHighlights()) flags |= ShowHighlights;
	setFlags(flags);
}

}
