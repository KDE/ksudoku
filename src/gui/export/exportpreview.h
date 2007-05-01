/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
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

#ifndef KSUDOKUEXPORTPREVIEW_H
#define KSUDOKUEXPORTPREVIEW_H

#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qmutex.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>

namespace ksudoku {

class ExportDlg;


/**
	Preview class used by ExportDlg
	Uses QPixmap as buffer. (Only calls for a redraw
	after a resizeEvent. Otherwise the old buffer is used)
 */
class ExportPreview : public QWidget
{
	Q_OBJECT
public:
	ExportPreview(ExportDlg const* eDlg, QWidget* parent)
		: QWidget(parent)
		, m_eDlg(eDlg)
	{
	}

public slots:
	///Calls @ref m_eDlg->draw() function to paint @ref m_qpixmap
	void draw();

protected:
	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);
private:
	///reference to external ExportDlg instance
	ExportDlg const* m_eDlg;
	///painter used for painting m_qpixmap
	QPainter m_qpainter;
	///QPixmap for buffering the content 
	///(contend is redrawn on resizeEvent
	/// other wise it is copied from m_qpixmap)
	QPixmap  m_qpixmap;
	
	///need mutex to avoid simultanious qpainter assignment
	///to m_qpainter
	QMutex m_mutex;
};

}

#endif
