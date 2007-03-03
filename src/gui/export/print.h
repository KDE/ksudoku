//
// C++ Interface: print
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
