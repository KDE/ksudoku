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

#ifndef _KSVIEW_H_
#define _KSVIEW_H_

#include "ksudokugame.h"


class QWidget;

namespace ksudoku {
	
enum ViewFlag {
	ShowErrors        = 0x01,
	ShowObviousErrors = 0x02,
	ShowHighlights    = 0x04
};

typedef QFlags<ViewFlag> ViewFlags;
	
class Game;
struct SymbolTable;
class ValueListWidget;
class GameActions;

/**
 * Every implementation of ViewInterface needs following signals:
 *   void valueSelected(int value);
 */
class ViewInterface {
public:
	virtual ~ViewInterface() {}
public:
	virtual QWidget* widget() = 0;
public: // SLOTS
	virtual void selectValue(int value) = 0;
};

/**
 * Interface for all views
 */
class KsView : public QObject
{
	Q_OBJECT
private:
	// prevent copy constructor (not implemented)
	KsView(KsView const& other);
	// prevent assignment (not implemented)
	KsView& operator=(KsView const& other);

public:
	KsView(const Game& game, GameActions* gameActions, QObject* parent = 0);
	virtual ~KsView();

	//getters
	///return game used by the view
	Game game() const { return m_game; }

	QWidget* widget() const { return m_viewWidget; }
	
	ValueListWidget* valueListWidget() const { return m_valueListWidget; }
	// TODO make this own the valueListWidget
	void setValueListWidget(ValueListWidget* widget) {
		m_valueListWidget = widget;
	}
	
	SymbolTable* symbolTable() const;
	void setSymbolTable(SymbolTable* table);
	
	ViewFlags flags() const { return m_flags; }
	void setFlags(ViewFlags flags) {
		m_flags = flags;
		emit flagsChanged(flags);
	}

public:
	void createView();
	
public slots:
	void selectValue(int value);
	
	void settingsChanged();
	
signals:
	void flagsChanged(ViewFlags flags);
	void symbolsChanged(SymbolTable* table);
	void valueSelected(int value);
	

private:
	void setWidget(QWidget* viewWidget);

private:
	///pointer to external Game
	Game m_game;

	SymbolTable* m_symbolTable;
	ViewFlags m_flags;

	GameActions* m_gameActions;
	
	ViewInterface* m_view;
	QWidget* m_viewWidget;
	ValueListWidget* m_valueListWidget;
	
	int m_currentValue;
};

}

#endif

