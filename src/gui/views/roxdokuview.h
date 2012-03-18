/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2008 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#ifndef ROXDOKUVIEW_h
#define ROXDOKUVIEW_h

#include <qgl.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QMouseEvent>
#include <QTimer>

#include "ArcBall.h"

#include "ksudokugame.h"
#include "ksview.h"

class SKGraph;

namespace ksudoku{

class Game;
class Symbols;

/**
 * GUI for a Roxdoku puzzle.
 */
class RoxdokuView : public QGLWidget, public ViewInterface
{
Q_OBJECT
public:
	RoxdokuView(ksudoku::Game game, Symbols* symbols, QWidget *parent = 0);
	~RoxdokuView();

	virtual QString status() const;

	void initializeGL();

	void resizeGL(int w, int h );

	QWidget* widget() { return this; }

public slots:
	void selectValue(int value);
	void settingsChanged();

signals:
	void valueSelected(int value); // Never used but connected to

protected:
	void paintGL();

	void myDrawCube(bool highlight, int cell,
			GLfloat x, GLfloat y, GLfloat z,
			bool outside);

	void Selection(int mouse_x, int mouse_y);
	void mouseReleaseEvent ( QMouseEvent * e ){
		if(e->button() == Qt::LeftButton) m_isClicked = false;
	}
	void mousePressEvent ( QMouseEvent * e ){
		if(e->button() == Qt::LeftButton) m_isClicked = true;
	}	
	void mouseMoveEvent(QMouseEvent* e) ;
	void mouseDoubleClickEvent(QMouseEvent* e);
	void wheelEvent (QWheelEvent* e){
		m_wheelmove += e->delta() * .02;
		updateGL();
	}

private slots:
	void delayOver();

private:
	void loadSettings();

	Symbols *    m_symbols;
	Game         m_game;
	SKGraph *    m_graph;

	int          m_base;
	int          m_order;
	int          m_size;
	int          m_width;
	int          m_height;
	int          m_depth;

	char         m_selected_number;

	ArcBallT *   m_arcBall;	
	bool         m_isClicked;
	bool         m_isRClicked;	
	bool         m_isDragging;	
	int          m_selection;
	int          m_lastSelection;
	QVector<int> m_highlights;

	float        m_dist;
	float        m_wheelmove;

	GLuint       m_texture[2][26];

	bool         m_guidedMode;
	bool         m_showHighlights;
	QTimer *     m_delayTimer;
	bool         m_timeDelay;
};

}

#endif
