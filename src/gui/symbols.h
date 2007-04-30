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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KSUDOKUSYMBOLS_H
#define KSUDOKUSYMBOLS_H


#include <QVector>
#include <QString>

namespace ksudoku {

typedef QVector<QChar> SymbolList;
enum SymbolType { numbers, letters, none};

/**
 * Provide (in a consistend way) the symbols for puzzles.
 * Provdes lookup to and from symbols used in the user interface
 * (eventually this class must provide in more symbols then the western
 *  alphanumeric characters)
 */
class Symbols{
//default will copy memberpointer which will be ok => no need
//	///prevent copy constructor
//	Symbols(Symbols const& other);
//	///prevent assignment
//	Symbols& operator=(Symbols const& other);
public:
	Symbols(bool autoType = true);
	~Symbols();

	///set type of symbols (numbers or chars)
	void setType(SymbolType symType);
	///choses symbol type according to order (order < 10 => numbers, else chars)
	void autoSetType();

	///return symbol at index @a index
	QChar const& symbol(int index) const { return m_symbolList[index]; }
	///overloaded index operator, same as @ref symbol(int index)
	QChar const& operator[](int index) const { return symbol(index); }

	///convert symbol to index number
	///@return -1 if s is not a valid symbol, else returns index number
	///@TODO check this for efficiency (if only used for lookup at user input current implementation is usable)
	int symbol2index(QChar const& s) const {
		SymbolList::const_iterator iter = m_symbolList.begin();
		SymbolList::const_iterator end  = m_symbolList.end  ();
		int i = 0;
		while(iter != end)
			if(*iter == s) return i;
			else           { ++i; ++iter; }
		return -1; //return error
	}
	//setters
	///set new order (number of elements in list)
	void setOrder(uint order);

	//getters
	///return current order (number of elements in list)
	uint order() const { return m_symbolList.size(); }
	
	///return current symbol type
	SymbolType const symbolType() const { return m_symbolType; }

	///return reference to current symbol list 
	///(list may change while using the result)
	SymbolList const& symbolList() const { return m_symbolList; }

private:
	///symbol generator, generates numbers
	QChar numberGenerator(int index);
	///symbol generator, generates letters
	QChar letterGenerator(int index);

	///function pointer to the current symbol generator
	QChar (Symbols::*symbolGenerator)(int index);
	///(re)fill the symbol list
	void fill();

	///if true, then autoSetType will be called when order changes
	bool m_autoType;

	///holds the symbols
	SymbolList m_symbolList;
	
	///current symbol type
	SymbolType m_symbolType;
};

}

#endif
