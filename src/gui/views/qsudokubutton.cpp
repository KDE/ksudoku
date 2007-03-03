// part of KSUDOKU - by Francesco Rossi <redsh@email.it> 2005

#include "qsudokubutton.h"
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

#include "ksudoku.h"
#include "ksudokugame.h"
#include "ksudokuview.h"

#include <kmessagebox.h>
#include <klocale.h>
#include "puzzle.h"

#include <iostream>

namespace ksudoku {

QSudokuButton::QSudokuButton(ksudokuView *parent, const char *name, int x, int y)
	: QWidget(parent)
	, m_ksView(*parent)
	, m_x(x)
	, m_y(y)
{
	m_needRedraw = true;

	m_mousein = false;
	m_state   = WrongValue;

	m_highlighted[0] = 0;
	m_highlighted[1] = 0;
	m_highlighted[2] = 0;
	m_highlighted[3] = 0;

	m_text = " ";

	setFocusPolicy(Qt::ClickFocus);
	m_connected=true;
	m_custom=false;
}

QSudokuButton::~QSudokuButton()
{
}


void QSudokuButton::resize()
{
// 	if(m_ksView.game().userPuzzle()->order == 0) m_ksView.game().order() = 9;
	int w = m_ksView.width () / m_ksView.game().puzzle()->solver()->g->sizeX();
	int h = m_ksView.height() / m_ksView.game().puzzle()->solver()->g->sizeY();
	setGeometry( m_x*(w), m_y*(h), w, h);

	//m_qpixmap = m_qpixmap.scaled(size()); //TODO destroy old one //TODO PORT
	m_needRedraw = true;//draw();
	update();
}

void QSudokuButton::enterEvent (QEvent *)
{
	if(!isConnected()) return;
	if(((ksudokuView*) &m_ksView)->getHighlighted() != -1)
	{
		emit finishHighlight();
		emit beginHighlight(m_ksView.game().value(m_x,m_y));
	}
	//printf(" (%d %d) %d %d - ", m_x, m_y, m_ksView.game().index(m_x,m_y),  ((GraphCustom*)m_ksView.game().puzzle()->solver()->g)->index(m_x, m_y));
	emit enter(m_x,m_y);
	m_mousein = true;
}

void QSudokuButton::focusOutEvent  (QEvent *) 
{//TODO it does not work
	if(!isConnected()) return;
	emit finishHighlight();
}

void QSudokuButton::leaveEvent (QEvent *) 
{
	if(!isConnected()) return;
	emit leave(m_x,m_y);
	m_mousein = false;
}

void QSudokuButton::exitEvent (QEvent *) 
{
	if(!isConnected()) return;
	m_mousein = false;
	emit enter(m_x,m_y);
}

void QSudokuButton::keyPressEvent ( QKeyEvent * e ) 
{
	if(!isConnected()) return;
	if(m_ksView.game().value(m_x,m_y) == 0) return;
	if(e->modifiers() & Qt::ShiftModifier)
		emit beginHighlight(m_ksView.game().value(m_x,m_y));
	e->ignore(); //pass on
}
	
void QSudokuButton::keyReleaseEvent ( QKeyEvent *e )
{
	if(!isConnected()) return;
	for(;;){ //hack to avoid goto's to one lable
	         //think about it and change your view on goto's !!
	         //(the loop never loops, but he, no goto's)

		emit finishHighlight();
		int key = e->key();

		switch(key){
			//process move (arrow) keys
			case Qt::Key_Left:
				emit enter(m_x-1, m_y);
				return;//key is processed, don't pass it on
			break;
			case Qt::Key_Up:
				emit enter(m_x, m_y-1);
				return;//key is processed, don't pass it on
			break;
			case Qt::Key_Right:
				emit enter(m_x+1, m_y);
				return;//key is processed, don't pass it on
			break;
			case Qt::Key_Down:
				emit enter(m_x, m_y+1);
				return;//key is processed, don't pass it on
			default:
				; //nothing, not processed (yet)
		}

		if(state() == GivenValue)
			break; //goto passOn;

		///@TODO make delete button user configurable
		//delete user input, if delete or backspace pressed
		if(key == Qt::Key_Delete || key == Qt::Key_Backspace){
			m_ksView.game().setValue(0, m_x, m_y);
			return; //key is processed, don't pass it on
		}

		int n = m_ksView.game().char2Value(e->text()[0].toAscii());
		if(n < 0)
			break; //goto passOn;

		if(m_ksView.isWaitingForNumber == -1)
		{
			if(e->modifiers() & Qt::AltModifier)
				m_ksView.game().setMarker(n, !m_ksView.game().marker(n, m_x, m_y), m_x, m_y);
			else
				m_ksView.game().setValue(n, m_x, m_y);
		} 
		else if(   m_ksView.isWaitingForNumber
		        == static_cast<int>(m_ksView.game().index(m_x,m_y)))
			m_ksView.game().setMarker(n, !m_ksView.game().marker(n, m_x, m_y), m_x, m_y);
		else
			break; //goto passOn;

		return; //key is processed, don't pass it on
	}

	//passOn: //jump to here when no more actions will be performed
	e->ignore();   //pass on
}

void QSudokuButton::paintEvent (QPaintEvent *)
{
	if(m_needRedraw)
	{
		QPainter p(this);
		draw(p);
	}
	//bitBlt(this, 0, 0, &m_qpixmap,0,0,-1,-1); //TODO PORT
	//p.drawPixmap(QPoint(0,0), &m_qpixmap);

}


void QSudokuButton::draw(QPainter& qpainter)
{
	qpainter.eraseRect(rect());

	if(isConnected())
 		if(m_ksView.isWaitingForNumber != static_cast<int>(m_ksView.game().index(m_x,m_y)))
			paintHighlight(qpainter);

	//draw border
	
	if(!isCustom())
	{
		QPen pen(QColor(150,150,150));
		pen.setWidth(1);
		qpainter.setPen(pen);
		qpainter.drawRect(rect());	
		drawMajorGrid(qpainter);
	}
	else if(isConnected())
	{
		GraphCustom* g = (GraphCustom*)m_ksView.game().puzzle()->solver()->g;
		int connections=0;
		int myIndex = g->index(m_x,m_y,0); //TODO m_x and m_y are swapped but there is definitely something that doesnt work with coordinates
		QPen pen(QColor(150,150,150));
		pen.setWidth(1);

		//LEFT
		if((m_y-1)>=0)
		{
			if(g->linksLeft[myIndex]!=0)
			{
				connections = 3-g->linksLeft[myIndex];
				if(connections<0) connections = 0;
				pen.setWidth( 1 + connections);
				pen.setColor(QColor(90/(connections+1),90/(connections+1),90/(connections+1)));
				qpainter.setPen(pen);
				qpainter.drawLine(0,0,width(),0);
			}
		}
		//TOP
		if((m_x-1)>=0)
		{
			if(g->linksUp[myIndex]!=0)
			{ 
				connections = 3-g->linksUp[myIndex];
				if(connections<0) connections = 0;
				pen.setWidth(1 + connections);
				pen.setColor(QColor(90/(connections+1),90/(connections+1),90/(connections+1)));
				qpainter.setPen(pen);
				qpainter.drawLine(0,0,0,height());
			}
		}
	}

	if(isConnected())
		drawValue    (qpainter);
}

void QSudokuButton::paintHighlight(QPainter& qpainter)
{
	if(m_ksView.highlighted == -1)
	{
		unsigned char cc[3][3] = {{0xf1,0x9e,0x53},{0x9b,0xcb,0xe8},{0xad,0xe8,0x9b}};
		unsigned int c[3]  = {0xdc,0xdc,0xdc};
		if(m_ksView.showTracker) 
		{
			for(int i=0; i < 3; ++i)
				if(m_highlighted[i] != 0)
					ITERATE(j,3)
						c[j] = static_cast<unsigned>(c[j]*.475f + cc[i][j]*.475f);
			if(m_highlighted[0]+m_highlighted[1]+m_highlighted[2] == 3)
				ITERATE(i,3)
					c[i] = static_cast<unsigned>(0xdc*.205f+cc[0][i]*.205f+cc[1][i]*.205f+cc[2][i]*.205f);
		}
		qpainter.fillRect(rect(), QBrush(QColor(c[0],c[1],c[2])));
	}
	else
//		if(m_highlighted[3]==1 || given == true)
		if(m_highlighted[3] == 1 || state() == GivenValue)
			qpainter.fillRect(rect(), QBrush(QColor(255,200,200)));
		else
			qpainter.fillRect(rect(), QBrush(QColor(0xdc,0xdc,0xdc)));
		
}

void QSudokuButton::drawMajorGrid(QPainter& qpainter)
{
	QPen pen(QColor(0,0,0));
	pen.setWidth(4);
	qpainter.setPen(pen);

	int puzzle_base = static_cast<int>(sqrt(m_ksView.game().order()));
	if(m_x%puzzle_base == puzzle_base-1)
		qpainter.drawLine(rect(). width(), 0, rect(). width(), rect().height());
	if(m_y%puzzle_base == puzzle_base-1)
		qpainter.drawLine(0, (int)rect().height(), (int)rect().width(), (int)rect().height());
}

void QSudokuButton::drawValue(QPainter& qpainter)
{
	bool marker = false;

	qpainter.setPen(QColor(100,100,100));
	switch(m_state) {
		case GivenValue:
			qpainter.setPen(QColor(0,0,100));
		break;
		case ObviouslyWrong:
		case WrongValue:
//		if(guidedMode == 1 && m_ksView.puzzle_mark_wrong) //else keep current pen
			if(m_ksView.m_guidedMode && m_ksView.game().puzzle()->hasSolution())
				qpainter.setPen(Qt::darkRed);
		break;
		case CorrectValue:
			//keep current, qpainter.setPen(QColor(125,125,125));
		break;
		case Marker:
			//keep current, qpainter.setPen(QColor(125,125,125));
			marker = true;
		break;
		default:
			KMessageBox::information(this, i18n("BUG: No default color defined, but it's apparently needed"));
	}
	
	QFont f; 
	Qt::AlignmentFlag align;
	if(marker) 
	{
		f.setPointSizeF(2.85*rect().height()/10);
		align = Qt::AlignRight;
	} else {
		f.setPointSizeF(4.0*rect().height()/10);
		align = Qt::AlignCenter;
	}
	
	qpainter.setFont(f);
	qpainter.drawText( rect(), align, m_text );
}


void QSudokuButton::mousePressEvent (QMouseEvent *mouseevent)
{
	if(!isConnected()) return;
	if(mouseevent->button() == Qt::LeftButton)
		emit clicked2 (m_x,m_y);
		
	if(mouseevent->button() == Qt::RightButton)
		emit rightclicked (m_x,m_y);
}


void QSudokuButton::updateData() {
	CellInfo info = m_ksView.game().cellInfo(m_x,m_y);

	m_state = info.state();
	switch(info.state()) {
		case GivenValue:
		case CorrectValue:
		case WrongValue:
		case ObviouslyWrong:
			if(!info.value()) {
				m_text = "";
			} else {
				m_text = m_ksView.game().value2Char(info.value());
			}
			break;
		case Marker:
			m_text = "";
			for(uint i = 0; i < m_ksView.game().order(); ++i) {
				if(info.marker(i+1))
					m_text += m_ksView.game().value2Char(i+1) + ' ';
			}
			break;
	}
	
	m_needRedraw = true;
	update();
}

}

#include "qsudokubutton.moc"

