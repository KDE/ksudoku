// part of KSudoku

#include "ksview.h"

#include "ksudokugame.h"

#include <qpixmap.h>
#include <qpainter.h>

namespace ksudoku{

KsView::KsView(QWidget* parent)
	:/* QWidget(parent)
	,*/ m_game()
{
}

KsView::~KsView()
{
}

void KsView::draw(QPainter& p, int height, int width) const
{ ///@TODO improve performance (low priority)
//	//get user view
//	QPixmap const qp(const_cast< KsView* >(this)->renderPixmap(width, height, FALSE));
//	//copy to QPainter
//	p.drawPixmap(0,0,qp,-1,-1);

}

}

//#include "ksview.moc"

