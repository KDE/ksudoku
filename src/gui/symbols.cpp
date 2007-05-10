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

#include "symbols.moc"

#include <KLocale>

namespace ksudoku {

///////////////////////////////////////////////////////////////////////////////
// struct SymbolTable
///////////////////////////////////////////////////////////////////////////////

SymbolTable::SymbolTable(const QString& name, const QString& text, const QVector<QChar>& symbols)
	: m_name(name), m_text(text), m_symbols(symbols)
{ }

int SymbolTable::maxValue() const { return m_symbols.size(); }
QChar SymbolTable::symbolForValue(int i) const {
	if(i <= 0 || i > maxValue()) return '\0';
	return m_symbols[i-1];
}
QString SymbolTable::name() const { return m_name; }
QString SymbolTable::text() const { return m_text; }

///////////////////////////////////////////////////////////////////////////////
// class Symbols
///////////////////////////////////////////////////////////////////////////////

Symbols::Symbols() {
	m_possibleTables << new SymbolTable("symbols", i18n("Simple Forms"), QVector<QChar>() << 0x2610 << 0x2606 << 0x2661 << 0x263E);
	m_possibleTables << new SymbolTable("dices", i18n("Dices"), QVector<QChar>() << 0x2680 << 0x2681 << 0x2682 << 0x2683 << 0x2684 << 0x2685);
	m_possibleTables << new SymbolTable("digits", i18n("Digits"), QVector<QChar>() << '1' << '2' << '3' << '4' << '5' << '6' << '7' << '8' << '9');
	m_possibleTables << new SymbolTable("letters_lower", i18n("Small Letters"), QVector<QChar>() << 'a' << 'b' << 'c' << 'd' << 'e' << 'f' << 'g' << 'h' << 'i' << 'j' << 'k' << 'l' << 'm' << 'n' << 'o' << 'p' << 'q' << 'r' << 's' << 't' << 'u' << 'v' << 'w' << 'x' << 'y' << 'z');
	m_possibleTables << new SymbolTable("letters_upper", i18n("Capital Letters"), QVector<QChar>() << 'A' << 'B' << 'C' << 'D' << 'E' << 'F' << 'G' << 'H' << 'I' << 'J' << 'K' << 'L' << 'M' << 'N' << 'O' << 'P' << 'Q' << 'R' << 'S' << 'T' << 'U' << 'V' << 'W' << 'X' << 'Y' << 'Z');
	
	m_enabledTables = m_possibleTables;
}

Symbols::~Symbols()
{
	qDeleteAll(m_possibleTables);
}

QList<SymbolTable*> Symbols::possibleTables() const {
	return m_possibleTables;
}

QStringList Symbols::enabledTables() const {
	QStringList tables;
	
	for(int i = 0; i < m_enabledTables.size(); ++i) {
		tables << m_enabledTables[i]->name();
	}
	
	return tables;
}

void Symbols::setEnabledTables(const QStringList& tables) {
	m_enabledTables.clear();
	
	for(int i = 0; i < m_possibleTables.size(); ++i) {
		SymbolTable* table = m_possibleTables[i];
		if(tables.contains(table->name()))
			m_enabledTables << table;
	}
	
	emit tablesChanged();
}

}
