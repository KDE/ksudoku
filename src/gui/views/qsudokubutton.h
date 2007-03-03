// part of KSUDOKU

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
	QSudokuButton(ksudokuView *parent = 0, const char *name = 0,   int x=0, int y=0);
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
	void setConnected(bool b){m_connected=b;};
	
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
	void clicked2(uint, uint);
	void enter   (uint, uint);
	void leave   (uint, uint);
	void rightclicked(uint, uint);
	
	void numberset(uint,uint,uint);

	void beginHighlight(uint val);
	void finishHighlight();

public:
	bool highlighted(int index) const { return m_highlighted[index]; }
	
	void setHighlighted(int index, bool state) 
		{ if(m_highlighted[index] == state) return; m_highlighted[index] = state; m_needRedraw = true;}
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

	bool m_highlighted[4];
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
