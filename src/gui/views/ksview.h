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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _KSVIEW_H_
#define _KSVIEW_H_

#include <qgl.h>
#include <qstring.h>

#include "ksudokugame.h"

namespace ksudoku {

class Game;

/**
 * Interface for all views
 */
class KsView // : public QWidget
{
//	Q_OBJECT
private:
	///prevent copy constructor (not implemented)
	KsView(KsView const& other);
	///prevent assignment (not implemented)
	KsView& operator=(KsView const& other);

public:
	KsView();
	virtual ~KsView();

	///draw content to external qpainter (use for printing etc.)
	///(if not reimplemented, a slow redraw to pixmap will be
	/// used (renderPixmap) and then copy it to a qpainter)
	virtual void draw(QPainter& p, int height, int width) const;

	//setters
	///@TODO document me
	virtual void setGame(const Game& game) =0;
	///set guidedMode. @see m_guidedMode
	void setGuidedMode(bool const mode) { m_guidedMode = mode; }

	///change guided state state (mark wrong entries red)
	void toggleGuided() { m_guidedMode = !m_guidedMode; }

	///@see m_guidedMode
	bool const guidedMode() const { return m_guidedMode; }

	//getters
	///return game used by the view
	Game game() const { return m_game; }

	///return some info on current status (can be used for status bar)
	virtual QString status() const =0;
	
	virtual QWidget* widget() = 0;

protected:
	///pointer to external Game
	Game m_game;

	///whether wrong entries (by user) should be visable color marked
	bool m_guidedMode;
};

}

#endif

