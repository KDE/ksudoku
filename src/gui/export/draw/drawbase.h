//
// C++ Interface: drawbase
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef KSUDOKUDRAWBASE_H
#define KSUDOKUDRAWBASE_H


class QPainter;

namespace ksudoku {

class Puzzle ;
class Symbols;


/**
 * Interface for all classes that draw a puzzle to a QPaintdevice
 */
class DrawBase{
public:
	DrawBase(Puzzle const& puzzle, Symbols const& symbols);
	virtual ~DrawBase();

	///draw game on QPainter device
	inline  void draw(QPainter& p, int width, int height) const;

	///draw raster on QPainter device
	virtual void drawRaster(QPainter& p, int width, int height) const = 0;
	///draw values on QPainter device
	virtual void drawValues(QPainter& p, int width, int height) const = 0;
	
protected:
	///reference to external game
	Puzzle const& m_puzzle;
	///reference to Symbols to use when exporting puzzle
	Symbols const& m_symbols;
};


void DrawBase::draw(QPainter& p, int width, int height) const{
	drawRaster(p,width,height);
	drawValues(p,width,height);
}


}

#endif
