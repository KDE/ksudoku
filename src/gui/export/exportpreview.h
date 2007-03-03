//
// C++ Interface: exportpreview
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
