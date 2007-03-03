//
// C++ Implementation: exportpreview
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

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
	bitBlt(this, 0, 0, &m_qpixmap,0,0,-1,-1);
	//QWidget::paintEvent(event);
}

void ExportPreview::draw()
{
	QMutexLocker locker(&m_mutex);

	m_qpixmap.resize(size());
	m_qpixmap.fill(Qt::black);

	m_qpainter.begin(&m_qpixmap);

	QSize ps = m_eDlg->currentPageSize();
	ps.scale(m_qpixmap.size(),QSize::ScaleMin);
	m_eDlg->draw(m_qpainter, ps.height(), ps.width());

	m_qpainter.end();
	
	update();
}

}

#include "exportpreview.moc"

