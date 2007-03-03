//
// C++ Interface: RoxdokuView
//
// Description: Part of KSudoku
//
// (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <qgl.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include "sudoku_solver.h"
#include "ArcBall.h"

#include "ksudokugame.h"
#include "ksview.h"


class KSudoku;

namespace ksudoku{

class Game;

/**
 * Gui for a roxdoku puzzle
 * @TODO hide private members (now public)
 */
class RoxdokuView : public QGLWidget, public ksudoku::KsView
{
Q_OBJECT
public:
//	RoxdokuView(QWidget *parent = 0, const char *name = 0, int order=9, int difficulty=1, int simmetry=0, bool dub=0);
	RoxdokuView(ksudoku::Game game, QWidget *parent = 0, const char* name = 0);
	~RoxdokuView();
public:
	///(re)implemented from KsView
	virtual void setGame(const ksudoku::Game& /* game*/) { /* ///@todo fixme */ };

	///(re)implemented from KsView
	virtual QString status() const;

	int base;
	int order;
	int size;
	char selected_number;

	bool isClicked;
	bool isRClicked;	
	bool isDragging;	
	ArcBallT*    ArcBall;	
	int selection;

	float dist;
	float wheelmove;

	GLuint  texture[2][26];

public:
	void initializeGL();

	void resizeGL( int w, int h ){ //hm this won't be compiled inline I think ..??
		if(w==0)w=1;	
		if(h==0)h=1;
		ArcBall = new ArcBallT((GLfloat)w,(GLfloat)h);

		glViewport( 0, 0, (GLint)w, (GLint)h );
		glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
		glLoadIdentity();            // Reset The Projection Matrix

		gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

		glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
		glLoadIdentity();
	}
protected:
	void paintGL();
protected:
	void Selection(int mouse_x, int mouse_y);
	void mouseReleaseEvent ( QMouseEvent * e ){
		if(e->button() == Qt::LeftButton) isClicked = false;
	}
	void mousePressEvent ( QMouseEvent * e ){
		if(e->button() == Qt::LeftButton) isClicked = true;
	}	
protected:
	void myDrawCube(int n, GLfloat x, GLfloat y, GLfloat z, int texture);
	void keyReleaseEvent(QKeyEvent* e);
	void mouseMoveEvent(QMouseEvent* e) ;
	void mouseDoubleClickEvent(QMouseEvent* e);
	void wheelEvent (QWheelEvent* e){
		wheelmove += e->delta() * .02;
		updateGL();
	}
};

}

#endif
