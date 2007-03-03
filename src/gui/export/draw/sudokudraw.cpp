//
// C++ Implementation: sudokudraw
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
