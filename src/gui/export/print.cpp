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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "print.h"
#include "printdialogpage.h"
#include "ksudokugame.h"
#include "ksview.h"

#include <kmessagebox.h>
#include <kprinter.h>
#include <q3paintdevicemetrics.h>

#include <kapplication.h>
#include <klocale.h>


namespace ksudoku {

Print::Print(KsView const& view)
	: m_view(view)
{
	toPrinter();
}


Print::~Print()
{
}


void Print::toPrinter(){
	KPrinter printer;
	printer.removeStandardPage(1); //there is only 1 standard page

	PrintDialogPage* pdp = new PrintDialogPage(*this);
	printer.addDialogPage(pdp);

	if (printer.setup())
	{
		Q3PaintDeviceMetrics metrics(&printer);
		float scale  = printer.option( SCALE ).toInt() / 100.0 ;
		float aspect = printer.option( ASPECT ).toFloat();

		QPainter p;
		p.begin(&printer);
		drawUsingPrinterSettings(p, scale, aspect, metrics.height(),metrics.width() );
		p.end();
	}
}

void Print::drawUsingPrinterSettings(QPainter& p, float scale, float aspect, int height, int width) const
{
 	int w = static_cast<int>(width  * scale);
	int h = static_cast<int>(height * scale);

	float viewAspectR = static_cast<float>(w) / h;
	float desiredAS = aspect;

	if( desiredAS > viewAspectR)
		h = static_cast<int>(w / desiredAS);
	else
		w = static_cast<int>(h* desiredAS);

//TODO PORT
//	m_view.draw(p, h, w);
}


}
