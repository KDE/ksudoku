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
