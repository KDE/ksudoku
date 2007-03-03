//
// C++ Implementation: drawbase
//
// Description: 
//
//
// Author:  <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "drawbase.h"


namespace ksudoku {

DrawBase::DrawBase(Puzzle const& puzzle, Symbols const& symbols)
	: m_puzzle(puzzle)
	, m_symbols(symbols)
{
}


DrawBase::~DrawBase()
{
}


}
