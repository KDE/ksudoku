/***************************************************************************
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KSUDOKUPRINT_H
#define KSUDOKUPRINT_H

#include <qpainter.h>


class KConfig;

namespace ksudoku {

class KsView;
class PrintDialogPage;
class DrawBase;

/**
 * export to printer
 */
class Print{
public:
	Print(KsView const& view);
	~Print();

	void toPrinter();

	///scale in %
 	void drawUsingPrinterSettings(QPainter& p, float scale, float aspect, int height, int width) const;

private:
	///Reference to external KsView
	KsView const& m_view;
};

}

#endif
