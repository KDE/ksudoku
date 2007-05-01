/***************************************************************************
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "sudokudraw.h"

//#include "ksudokugame.h"
#include "puzzle.h"
#include "sudoku_solver.h"
#include "symbols.h"

#include <qpainter.h>
#include <math.h>


namespace ksudoku {

SudokuDraw::SudokuDraw(Puzzle const& puzzle, Symbols const& symbols)
 : DrawBase(puzzle, symbols)
{
}


SudokuDraw::~SudokuDraw()
{
}


void SudokuDraw::drawRaster(QPainter& p, int width, int height) const
{
	int wStep = width  / m_puzzle.order();
	int hStep = height / m_puzzle.order();
	width  = wStep * m_puzzle.order(); //trunk widht and height to 
	height = hStep * m_puzzle.order(); // multiple of Step

	QPen pen(QColor(50,50,50));
	for(uint i=0; i <= m_puzzle.order(); ++i)
	{
		uint base = (uint)sqrt(m_puzzle.order());
		
		pen.setWidth( (i%base==0) ? 4 : 1 );
		p.setPen(pen);
		
		//draw lines from (x1,y1)  to  (x2,y2)
		p.drawLine(0, wStep*(i),  height, wStep*(i)); //hLines
		p.drawLine(hStep*(i), 0,  hStep*(i), width ); //vLines
	}
}

void SudokuDraw::drawValues(QPainter& p, int width, int height) const
{
	int wStep = width  / m_puzzle.order();
	int hStep = height / m_puzzle.order();
	width  = wStep * m_puzzle.order(); //trunk widht and height to 
	height = hStep * m_puzzle.order(); // multiple of Step

	QFont f;
	float maxFS = ((height < width) ? height : width) / static_cast<float>(m_puzzle.order());
	f.setPointSizeFloat(maxFS * 0.7); //max 70% font size
	p.setFont(f);

	for(unsigned x=0; x < m_puzzle.order(); ++x)
		for(unsigned y=0; y < m_puzzle.order(); ++y)
			if(m_puzzle.value(x,y) != 0)
			{
				p.drawText( x*hStep, y*wStep, hStep, wStep
				          , QPainter::AlignCenter, m_symbols[m_puzzle.value(x,y)-1] //-1 because 0 is used by puzzle as special vaule
				          );
			}
}

}
