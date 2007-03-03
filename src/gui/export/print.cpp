//
// C++ Implementation: print
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

	m_view.draw(p, h, w);
}


}
