//
// C++ Implementation: symbols
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "symbols.h"



namespace ksudoku {


Symbols::Symbols(bool autoType)
	: symbolGenerator(0)
	, m_autoType(autoType)
	, m_symbolType(none)
{
}


Symbols::~Symbols()
{
}

QChar Symbols::numberGenerator(int index)

{
	///@TODO fix this for internationalisation
	return QChar(index + '1');
}
QChar Symbols::letterGenerator(int index)
{
	///@TODO fix this for internationalisation
	return QChar(index + 'a');
}

void Symbols::setOrder(uint order)
{
	if(Symbols::order() == order)
		return;

	m_symbolList.resize(order);
	if(m_autoType)
		autoSetType();
	fill();
}

void Symbols::setType(SymbolType symType)
{
	if(symType == m_symbolType)
		return; //nothing to do

	m_symbolType = symType;

	switch(m_symbolType){
		case numbers:
			symbolGenerator = &Symbols::numberGenerator;
		break;
		case letters:
			symbolGenerator = &Symbols::letterGenerator;
		break;
		default: //none
			symbolGenerator = 0;
	};

	fill(); //(re)fill symbol list
}

void Symbols::autoSetType(){
	if(order() < 10)
		setType(numbers);
	else
		setType(letters);
}

void Symbols::fill()
{
	if(m_symbolType == none)
		autoSetType(); //call setType(<SymbolType>) first
		               //now just use autoSetTye as best practice

	int capacity = m_symbolList.size();
	m_symbolList.clear();
	for( int i=0; i < capacity ; ++i){
		m_symbolList.push_back((this->*symbolGenerator)(i));
	}
}

}
