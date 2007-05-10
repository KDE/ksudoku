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

#ifndef KSUDOKUSYMBOLS_H
#define KSUDOKUSYMBOLS_H


#include <QVector>
#include <QString>
#include <QObject>

namespace ksudoku {

struct SymbolTable {
public:
	SymbolTable(const QString& name, const QString& text, const QVector<QChar>& symbols);
public:
	int maxValue() const;
	QChar symbolForValue(int i) const;
	QString name() const;
	QString text() const;
private:
	QString m_name;
	QString m_text;
	QVector<QChar> m_symbols;
};

/**
 * Provides the symbols for puzzles.
 */
class Symbols : public QObject {
Q_OBJECT
public:
	Symbols();
	~Symbols();

	QList<SymbolTable*> possibleTables() const;
	
	QStringList enabledTables() const;
	void setEnabledTables(const QStringList& tables);
	
	SymbolTable* selectTable(int maxValue) {
		for(int i = 0; i < m_enabledTables.size(); ++i) {
			SymbolTable* table = m_enabledTables[i];
			if(table->maxValue() >= maxValue)
				return table;
		}
		return 0;
	}

	QChar value2Symbol(int value, int maxValue) {
		SymbolTable* table = selectTable(maxValue);
		if(!table) return '\0';
		return table->symbolForValue(value);
	}

	/// returns the symbol vor a value used for loading and saving
	static QChar ioValue2Symbol(int value) {
		if(value <= 0) return '_';
		return 'a' + value;
	}
	
	/// returns the number of the index
	static int ioSymbol2Value(const QChar& symbol) {
		char c = symbol.toAscii();
		if(symbol == '_') return 0;
		return c - 'a';
	}

signals:
	void tablesChanged();

private:
	QList<SymbolTable*> m_possibleTables;
	QList<SymbolTable*> m_enabledTables;
};

}

#endif
