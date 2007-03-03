//
// C++ Interface: sudokudraw
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef KSUDOKUSUDOKUDRAW_H
#define KSUDOKUSUDOKUDRAW_H

#include "drawbase.h"

namespace ksudoku {

/**
 * Draw sudoku game to QPaintdevice
 */
class SudokuDraw : public DrawBase
{
public:
	SudokuDraw(Puzzle const& puzzle, Symbols const& symbols);
	~SudokuDraw();

	void drawRaster(QPainter& p, int width, int height) const;
	void drawValues(QPainter& p, int width, int height) const;
};

}

#endif
