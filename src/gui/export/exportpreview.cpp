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

#include "exportpreview.h"

#include "exportdlg.h"
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>


namespace ksudoku {


void ExportPreview::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	draw();
}

void ExportPreview::paintEvent(QPaintEvent* /*event*/)
{
//TODO PORT
//	bitBlt(this, 0, 0, &m_qpixmap,0,0,-1,-1);
	//QWidget::paintEvent(event);
}

void ExportPreview::draw()
{
	QMutexLocker locker(&m_mutex);
//TODO PORT
//	m_qpixmap.resize(size());
	m_qpixmap.fill(Qt::black);

	m_qpainter.begin(&m_qpixmap);

	QSize ps = m_eDlg->currentPageSize();
//TODO PORT
//	ps.scale(m_qpixmap.size(),QSize::ScaleMin);
	m_eDlg->draw(m_qpainter, ps.height(), ps.width());

	m_qpainter.end();
	
	update();
}

}

#include "exportpreview.moc"

