/***************************************************************************
 *   Copyright 2005-2007 Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006-2007 Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include "qsudokubutton.h"
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

#include <QLinearGradient>

#include "ksudoku.h"
#include "ksudokugame.h"
#include "sudokuview.h"

#include <kmessagebox.h>
#include <klocale.h>
#include "puzzle.h"

#include <iostream>

namespace ksudoku {

// Colors of kde4.0alpha and first betas
/* enum GameColors {
	// Standard background
	HColorStd = 0xffbfd9ff,

	// Pure highlights
	HColorR = 0xff80b3ff, // Row highlight
	HColorC = 0xfff082b0, // Column highlight
	HColorB = 0xff00cc88, // Block highlight

	// Combinations of highlights
	HColorRC  = 0xffe8b7d7,
	HColorRB  = 0xff6193cf,
	HColorCB  = 0xffb3925d,
	HColorRCB = 0xff888a85,

	// Special highlights
	HColorSStd = HColorStd,
	HColorSR   = HColorR,
	HColorSC   = HColorC,
	HColorSB   = HColorB,
	HColorSRC  = HColorRC,
	HColorSRB  = HColorRB,
	HColorSCB  = HColorCB,
	HColorSRCB = HColorRCB,

	// Highlights for the helper
	HColorGood  = 0xffbfffbf,
	HColorBad   = 0xffffbfbf,
 	HColorSGood = HColorGood,
	HColorSBad  = HColorBad,

	// Border colors
	BColorDark   = 0xff555753,
	BColorLight  = 0xffffffff, // 0xeeeeec
	BColorSimple = 0xff888a85,

	// Font Colors
	FColorStd    = 0xff646464,
	FColorGiven  = 0xff000064,
	FColorWrong  = 0xff800000,
	FColorMarker = 0xff646464
}; */

// Almost Flat Strong colors
enum GameColors {
	// Standard background
	HColorStd = 0xffffffff,

	// Pure highlights
	HColorR = 0xffff8080, // Row highlight
	HColorC = 0xff6192cf, // Column highlight
	HColorB = 0xfffff199, // Block highlight

	// Combinations of highlights
	HColorRC  = 0xffc173b0,
	HColorRB  = 0xfff2bb88,
	HColorCB  = 0xff00cc88,
	HColorRCB = 0xffbabdb6,

	// Special highlights
	HColorSStd = 0xffd3d7cf,
	HColorSR   = 0xffe85752,
	HColorSC   = 0xff2c72c7,
	HColorSB   = 0xffffeb55,
	HColorSRC  = 0xffb14f9a,
	HColorSRB  = 0xfff29b68,
	HColorSCB  = 0xff00b377,
	HColorSRCB = 0xff888a85,

	// Highlights for the helper
	HColorGood  = 0xffbfffbf,
	HColorBad   = 0xffffbfbf,
	HColorSGood = 0xff80ff80,
	HColorSBad  = 0xffff8080,

	// Border colors
	BColorDark   = 0xff555753,
	BColorLight  = 0xff555753,
	BColorSimple = 0xff555753,

	// Font Colors
	FColorStd    = 0xff646464,
	FColorGiven  = 0xff000064,
	FColorWrong  = 0xff800000,
	FColorMarker = 0xff646464
};

// With 3D look
/* enum GameColors {
	// Standard background
	HColorStd = 0xffdddddd,

	// Pure highlights
	HColorR = 0xffffbfbf, // Row highlight
	HColorC = 0xffa4c0e4, // Column highlight
	HColorB = 0xfffff6c8, // Block highlight

	// Combinations of highlights
	HColorRC  = 0xffe8b7d7,
	HColorRB  = 0xfffcd8b0,
	HColorCB  = 0xff99dcc6,
	HColorRCB = 0xffbabdb6,

	// Special highlights
	HColorSStd = HColorStd,
	HColorSR   = HColorR,
	HColorSC   = HColorC,
	HColorSB   = HColorB,
	HColorSRC  = HColorRC,
	HColorSRB  = HColorRB,
	HColorSCB  = HColorCB,
	HColorSRCB = HColorRCB,

	// Highlights for the helper
	HColorGood  = 0xffbfffbf,
	HColorBad   = 0xffffbfbf,
 	HColorSGood = HColorGood,
	HColorSBad  = HColorBad,

	// Border colors
	BColorDark   = 0xff888a85,
	BColorLight  = 0xffeeeeee, // 0xeeeeec
	BColorSimple = 0xff888a85,

	// Font Colors
	FColorStd    = 0xff646464,
	FColorGiven  = 0xff000064,
	FColorWrong  = 0xff800000,
	FColorMarker = 0xff646464
}; */

// Lookup for Highlights
// Bit 1: Horz clique highlight
// Bit 2: Vert clique highlight
// Bit 3: Other Clique highlight
// Bit 4: Use Help Highlights
// Bit 5: Helper Highlight Good/Bad
// Bit 6: Special highlights (e.g. cross in XSudoku)
const uint highlightColors[] = {
	HColorStd,   HColorR,     HColorC,     HColorRC,
	HColorB,     HColorRB,    HColorCB,    HColorRCB,
	HColorBad,   HColorBad,   HColorBad,   HColorBad,
	HColorBad,   HColorBad,   HColorBad,   HColorBad,

	HColorStd,   HColorR,     HColorC,     HColorRC,
	HColorB,     HColorRB,    HColorCB,    HColorRCB,
	HColorGood,  HColorGood,  HColorGood,  HColorGood,
	HColorGood,  HColorGood,  HColorGood,  HColorGood,

	HColorSStd,  HColorSR,    HColorSC,    HColorSRC,
	HColorSB,    HColorSRB,   HColorSCB,   HColorSRCB,
	HColorSBad,  HColorSBad,  HColorSBad,  HColorSBad,
	HColorSBad,  HColorSBad,  HColorSBad,  HColorSBad,

	HColorSStd,  HColorSR,    HColorSC,    HColorSRC,
	HColorSB,    HColorSRB,   HColorSCB,   HColorSRCB,
	HColorSGood, HColorSGood, HColorSGood, HColorSGood,
	HColorSGood, HColorSGood, HColorSGood, HColorSGood,
};

QSudokuButton::QSudokuButton(SudokuView *parent, int x, int y)
	: QWidget(parent)
	, m_ksView(*parent)
	, m_x(x)
	, m_y(y)
{
	m_needRedraw = true;

	m_mousein = false;
	m_state   = WrongValue;

	m_highlights = HighlightNone;

	// make cells surrounded with big borders special
	Graph2d* g = dynamic_cast<Graph2d*>(m_ksView.game().puzzle()->solver()->g);
	if(g &&
	   g->hasLeftBorder(m_x, m_y) &&
	   g->hasTopBorder(m_x, m_y) &&
	   g->hasRightBorder(m_x, m_y) &&
	   g->hasBottomBorder(m_x, m_y))
	{
		m_highlights = HighlightSpecial;
	}

	m_text = " ";

	setFocusPolicy(Qt::ClickFocus);
	m_custom=false;

	setAttribute(Qt::WA_OpaquePaintEvent, true);
}

QSudokuButton::~QSudokuButton()
{
}


void QSudokuButton::resize()
{
	int w = m_ksView.width () / m_ksView.game().puzzle()->solver()->g->sizeX();
	int h = m_ksView.height() / m_ksView.game().puzzle()->solver()->g->sizeY();
	setGeometry( m_x*(w), m_y*(h), w, h);

	//m_qpixmap = m_qpixmap.scaled(size()); //TODO destroy old one //TODO PORT
	m_needRedraw = true;//draw();

	updateData();
}

void QSudokuButton::enterEvent (QEvent *)
{
	emit enter(m_x,m_y);
	m_mousein = true;
}

void QSudokuButton::focusOutEvent  (QEvent *)
{//TODO it does not work
	emit finishHighlight();
}

void QSudokuButton::leaveEvent (QEvent *)
{
	emit leave(m_x,m_y);
	m_mousein = false;
}

void QSudokuButton::exitEvent (QEvent *)
{
	m_mousein = false;
	emit enter(m_x,m_y);
}

void QSudokuButton::keyPressEvent ( QKeyEvent * e )
{
	if(e->modifiers() & Qt::ControlModifier)
		emit beginHighlight();
	e->ignore(); //pass on
}

void QSudokuButton::keyReleaseEvent ( QKeyEvent *e )
{
	emit finishHighlight();
	e->ignore();
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

	paintHighlight(qpainter);


	Graph2d* g = dynamic_cast<Graph2d*>(m_ksView.game().puzzle()->solver()->g);
	if(!g) return;

	//draw border
	bool left = g->hasLeftBorder(m_x, m_y);
	bool top = g->hasTopBorder(m_x, m_y);
	bool right = g->hasRightBorder(m_x, m_y);
	bool bottom = g->hasBottomBorder(m_x, m_y);
	//LEFT
	QPen pen;
	if(!(top && left && bottom && right)) {
		pen.setWidth( 1 );
		pen.setColor(QColor(BColorSimple));
		qpainter.setPen(pen);
		if(!top) { qpainter.drawLine(0,0,width()-1,0); }
		if(!left) { qpainter.drawLine(0,0,0,height()-1); }
		if(!bottom) { qpainter.drawLine(0,height()-1,width()-1,height()-1); }
		if(!right) { qpainter.drawLine(width()-1,0,width()-1,height()-1); }
	}

	if(top || left) {
		pen.setWidth( 5 );
		pen.setColor(QColor(BColorDark));
		qpainter.setPen(pen);
		if(top) { qpainter.drawLine(0,0,width()-1,0); }
		if(left) { qpainter.drawLine(0,0,0,height()-1); }
	}
	if(bottom || right) {
		pen.setWidth( 5 );
		pen.setColor(QColor(BColorLight));
		qpainter.setPen(pen);
		if(bottom) { qpainter.drawLine(0,height()-1,width()-1,height()-1); }
		if(right) { qpainter.drawLine(width()-1,0,width()-1,height()-1); }
	}

	drawValue    (qpainter);
}

void QSudokuButton::paintHighlight(QPainter& qpainter) {
	if(0xffffffff != highlightColors[m_highlights & HighlightMask]) {
		Graph2d* g = dynamic_cast<Graph2d*>(m_ksView.game().puzzle()->solver()->g);
		if(!g) return;
		bool left = g->hasLeftBorder(m_x, m_y);
		bool top = g->hasTopBorder(m_x, m_y);
		bool right = g->hasRightBorder(m_x, m_y);
		bool bottom = g->hasBottomBorder(m_x, m_y);

		QRect pos = rect();
		QLinearGradient grad(0,-pos.height(),pos.width(),pos.height());
		grad.setColorAt(0.0, 0xffffffff);
		grad.setColorAt(1.0, highlightColors[m_highlights & HighlightMask]);
		qpainter.fillRect(pos.adjusted(left?3:1, top?3:1, right?-3:-1, bottom?-3:-1), grad);
	} else {
		qpainter.fillRect(rect(), QBrush(highlightColors[m_highlights & HighlightMask]));
	}
}

void QSudokuButton::drawValue(QPainter& qpainter)
{
	bool marker = false;

	qpainter.setPen(FColorStd);
	switch(m_state) {
		case GivenValue:
			qpainter.setPen(FColorGiven);
			break;
		case ObviouslyWrong:
		case WrongValue:
			if(m_ksView.m_flags.testFlag(ShowErrors))
				qpainter.setPen(FColorWrong);
			break;
		case CorrectValue:
			break;
		case Marker:
			qpainter.setPen(FColorMarker);
			marker = true;
			break;
		default:
			KMessageBox::information(this, i18n("BUG: No default color defined, but it is apparently needed"));
	}

	QTextOption textOption(Qt::AlignCenter);

	if(marker)
	{
		if(m_cols > 0) {
			QFontMetrics fm(m_font);
			uint cwidth = fm.width(QChar('0'));

			QList<qreal> tabs;
			for(int i = 0; i < m_cols; ++i) {
				tabs.append((i+1)*rect().width()/(m_cols+1) -cwidth/2);
			}
			textOption.setTabArray(tabs);
			textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		} else {
			textOption.setAlignment(Qt::AlignRight);
		}
	}

	qpainter.setFont(m_font);
	qpainter.drawText( rect(), m_text, textOption);
}


void QSudokuButton::mousePressEvent (QMouseEvent *mouseevent)
{
	if(mouseevent->button() == Qt::LeftButton)
		emit clicked2 (m_x,m_y);

	if(mouseevent->button() == Qt::RightButton)
		emit rightclicked (m_x,m_y);
}


void QSudokuButton::updateData() {
	CellInfo info = m_ksView.game().cellInfo(m_x,m_y);

	if(m_ksView.symbol(1) == '\0') {
		m_state = ObviouslyWrong;
		m_text = "?";
		return;
	}

	int width = rect().width();
	int height = rect().height();

	m_state = info.state();
	switch(info.state()) {
		case GivenValue:
		case CorrectValue:
		case WrongValue:
		case ObviouslyWrong:
			if(rect().height() <= 2) {
				m_text = QString();
				break;
			}

			if(!info.value()) {
				m_text = QString();
			} else {
				m_text = m_ksView.symbol(info.value());
			}

			m_font.setPointSizeF(qMin(0.4 * height, 0.4*width));
			break;
		case Marker:
			m_text = QString();
			if(rect().height() <= 4) break;

			// Returns the width
			int order = m_ksView.game().order();
			int entries = order;
			if(order > 16) entries = info.markers().count(true);
			m_cols = qMax(2, int(sqrt(entries-1))+1);
			m_rows = qMax(2, (entries-1) / width +1);

			if(entries == order) {
				// Align markers in a grid. Each value has its own cell
				// not depending on whether it has a marker

				for(int i = 0; i < order; ++i) {
					m_text += '\t';
					if(info.marker(i+1))
						m_text += m_ksView.symbol(i+1);
					else
						m_text += ' ';

					if(!((i+1) % m_cols) && (i+1 != order)) {
						m_text += '\n';
					}
				}

				m_font.setPointSizeF(qMin(0.35 * height/m_rows, 0.6 * width/m_cols));
			} else if (m_cols <= 4) {
				// Align markers in a grid. Only markers get a cell
				int count = 0;
				for(int i = 0; i < order; ++i) {
					if(info.marker(i+1)) {
						m_text += '\t';
						m_text += m_ksView.symbol(i+1);
						if(!(++count % m_cols) && (count != entries)) {
							m_text += '\n';
						}
					}
				}

				m_font.setPointSizeF(qMin(0.35 * height/m_rows, 0.6 * width/m_cols));
			} else {
				// Try to show markers as free text

				m_cols = -1;
				// TODO implement this
			}
			break;
	}

	m_needRedraw = true;
	update();
}

bool QSudokuButton::hasHighlight(int mask) const {
	return ((m_highlights & mask) == mask);
}

void QSudokuButton::setHighlight(int value) {
	int oldValue = m_highlights;
	m_highlights |= value;

	if(highlightColors[m_highlights & HighlightMask] != highlightColors[oldValue & HighlightMask])
		update();
}

void QSudokuButton::setHighlight(int mask, int value) {
	int oldValue = m_highlights;
	m_highlights = m_highlights & (HighlightMask ^ mask) | value;

	if(highlightColors[m_highlights & HighlightMask] != highlightColors[oldValue & HighlightMask])
		update();
}



}

#include "qsudokubutton.moc"

