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

#ifndef _KSVIEW_H_
#define _KSVIEW_H_

#include <qgl.h>
#include <qstring.h>

#include "ksudokugame.h"

namespace ksudoku {
	
enum ViewFlag {
	ShowErrors        = 0x01,
	ShowObviousErrors = 0x02,
	ShowTracker       = 0x04,
};

typedef QFlags<ViewFlag> ViewFlags;
	
class Game;
class SymbolTable;

/**
 * Interface for all views
 */
class KsView : public QObject
{
	Q_OBJECT
private:
	///prevent copy constructor (not implemented)
	KsView(KsView const& other);
	///prevent assignment (not implemented)
	KsView& operator=(KsView const& other);

public:
	KsView(const Game& game, QObject* parent = 0);
	virtual ~KsView();

	///draw content to external qpainter (use for printing etc.)
	///(if not reimplemented, a slow redraw to pixmap will be
	/// used (renderPixmap) and then copy it to a qpainter)
	virtual void draw(QPainter& p, int height, int width) const;

	//getters
	///return game used by the view
	Game game() const { return m_game; }

	QWidget* widget() const { return m_viewWidget; }
	void setWidget(QWidget* viewWidget);
	
	
	SymbolTable* symbolTable() const;
	void setSymbolTable(SymbolTable* table);
	
	ViewFlags flags() const { return m_flags; }
	void setFlags(ViewFlags flags) {
		m_flags = flags;
		emit flagsChanged(flags);
	}
	
public slots:
	void setCursor(int cell);
	void selectValue(int value);
	void enterValue(int value);
	void markValue(int value);
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
	
signals:
	void flagsChanged(ViewFlags flags);
	void symbolsChanged(SymbolTable* table);
	void cursorMoved(int cell);
	void valueSelected(int value);
	
protected:
	///pointer to external Game
	Game m_game;

	SymbolTable* m_symbolTable;
	ViewFlags m_flags;
	
	QWidget* m_viewWidget;
	
	int m_currentValue;
	int m_currentCell;
};

}

#endif

