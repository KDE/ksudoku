//
// C++ Interface: ksview
//
// Description: Part of KSudoku
//
// (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

