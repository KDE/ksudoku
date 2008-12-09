/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *   Copyright 2007      Mick Kappenburg <ksudoku@kappendburg.net>         *
 *   Copyright 2007      Johannes Bergmeier <Johannes.Bergmeier@gmx.net>   +
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

#include <QtCore/QChar>

namespace ksudoku {

/// returns the symbol vor a value used for loading and saving
QChar Symbols::ioValue2Symbol(int value) {
	if(value <= 0) return '_';
	return 'a' + value;
}

/// returns the number of the index
int Symbols::ioSymbol2Value(const QChar& symbol) {
	char c = symbol.toAscii();
	if(symbol == '_') return 0;
	return c - 'a';
}

}
