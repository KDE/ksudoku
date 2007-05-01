/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2007      Mick Kappenburg <ksudoku@kappendburg.net>         *
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
	///@TODO fix this for internationalization
	return QChar(index + '1');
}
QChar Symbols::letterGenerator(int index)
{
	///@TODO fix this for internationalization
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
