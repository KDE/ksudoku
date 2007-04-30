/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2007      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#ifndef QSUDOKUBUTTON_H
#define QSUDOKUBUTTON_H


#include "ksudoku_types.h"

#include <qwidget.h>
#include <qpixmap.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>

class QPaintEvent;

namespace ksudoku {

class ksudokuView;

enum HighlightValues {
	HighlightRow        = 0x01, // for rows
	HighlightColumn     = 0x02, // for columns
	HighlightClique     = 0x04, // for other cliques
	HighlightShowHelp   = 0x08, // set this when help (shift-key) is to be shown
	HighlightHelpGreen  = 0x10, // for cells which are valid for current number
	HighlightSpecial    = 0x20, // for special cells or special cliques (not used yet)

	HighlightNone       = 0x00,
	HighlightBaseMask   = 0x07, // mask for row, column and other cliques
	HighlightHelpMask   = 0x18,
	HighlightMask       = 0x3f
};

/**
 * QSudokuButton represents a tile in KSudokuView
 */
class QSudokuButton : public QWidget
{
Q_OBJECT
private:
	///prevent copy constructor
	QSudokuButton(QSudokuButton const& other);
	///prevent assignment
	QSudokuButton& operator=(QSudokuButton const& other);

public:
	explicit QSudokuButton(ksudokuView *parent = 0, int x=0, int y=0);
	~QSudokuButton();

	void resize();
	void paintEvent (QPaintEvent *); //2FIX
	void mousePressEvent (QMouseEvent *);
	void enterEvent(QEvent*);
	void exitEvent (QEvent*);
	void leaveEvent(QEvent*);
	void keyReleaseEvent( QKeyEvent* e );
	void focusOutEvent  (QEvent *);
	void keyPressEvent  ( QKeyEvent* e );
	
	void updateData();

	bool isConnected(){return m_connected;}
	void setConnected(bool b);
	
	bool isCustom(){return m_custom;}
	void setCustom(bool b){m_custom=b;};
	

	inline ksudoku::ButtonState state() const { return m_state; }

public slots:
	///this repaints the widget. (repaint would be a better name
	///but is taken by QWidget for other purposes)
	void draw() { QPainter p(&m_qpixmap); draw(p); p.end();
	              m_needRedraw = false; }

	///Draw content to external QPainter. Painter should be
	///open and will be left open
	void drawExt(QPainter& p) { draw(p); }

	
signals:
	void clicked2(int, int);
	void enter   (int, int);
	void leave   (int, int);
	void rightclicked(int, int);
	
	void numberset(int,int,int);

	void beginHighlight(int val);
	void finishHighlight();

public:
	bool hasHighlight(int mask) const;
	
	void setHighlight(int value);
	void setHighlight(int mask, int value);
	void setX(int x) { if(m_x == x) return; m_x = x; m_needRedraw = true; }
	void setY(int y) { if(m_y == y) return; m_y = y; m_needRedraw = true; }
	int getX() { return m_x; }
	int getY() { return m_y; }

private:
	///draw content to qpainter device
	
	void draw(QPainter& qpainter);
	///responsable for expressing hightlighting (if needed)
	void paintHighlight(QPainter& qpainter);
	///responsable for creating the major grid lines
	void drawMajorGrid(QPainter& qpainter);
	///responsable for showing the value (if available)
	void drawValue(QPainter& painter);


	///reference to ksudokuView parent
	///(could use parent, but this makes the code a bit
	/// more readable)
	ksudokuView& m_ksView;

	uint m_highlights;
	
	int  m_x;
	int  m_y;
	bool m_mousein;
	QString m_text;
	bool m_connected;
	bool m_custom;
	ksudoku::ButtonState m_state;

	///QPixmap for buffering the content 
	///(contend is redrawn on resizeEvent and ??
	/// other wise it is copied from m_qpixmap)
	QPixmap  m_qpixmap;
	///if true draw() will be called at next paintEvent
	bool m_needRedraw;
};

}

#endif
