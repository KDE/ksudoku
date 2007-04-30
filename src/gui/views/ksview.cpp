/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2006-2007 Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2006      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ksview.h"

#include "ksudokugame.h"

#include <qpixmap.h>
#include <qpainter.h>

namespace ksudoku{

KsView::KsView()
	: m_game()
{
}

KsView::~KsView()
{
}

void KsView::draw(QPainter& /*p*/, int /*height*/, int /*width*/) const
{ ///@TODO improve performance (low priority)
//	//get user view
//	QPixmap const qp(const_cast< KsView* >(this)->renderPixmap(width, height, FALSE));
//	//copy to QPainter
//	p.drawPixmap(0,0,qp,-1,-1);

}

}
