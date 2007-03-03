//
// C++ Interface: roxdokudraw
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KSUDOKUROXDOKUDRAW_H
#define KSUDOKUROXDOKUDRAW_H

#include "drawbase.h"

namespace ksudoku {

/**
 * Draw roxdoku game to QPaintdevice
 */
class RoxdokuDraw : public DrawBase
{
public:
	RoxdokuDraw(Puzzle const& puzzle, Symbols const& symbols);
	~RoxdokuDraw();

	///@TODO implement me
	void drawRaster(QPainter& p, int width, int height) const {;}
	///@TODO implement me
	void drawValues(QPainter& p, int width, int height) const {;}
};

}

#endif
