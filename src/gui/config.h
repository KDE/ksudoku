/***************************************************************************
 *   Copyright 2007      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#ifndef _KSUDOKUCONFIG_H_
#define _KSUDOKUCONFIG_H_

#include <QWidget>
#include <QList>
#include <QListWidget>

#include "ui_configgame.h"

namespace ksudoku {

class Symbols;
struct SymbolTable;

class SymbolConfigListWidget : public QListWidget {
Q_OBJECT
Q_PROPERTY(QStringList enabledTables READ enabledTables WRITE setEnabledTables USER true)

public:
	SymbolConfigListWidget(const QList<SymbolTable*>& tables, QWidget* parent = 0);
	~SymbolConfigListWidget();

public:
	QStringList enabledTables() const;
	void setEnabledTables(const QStringList& tables);
};

class SymbolConfig : public QWidget {
Q_OBJECT
public:
	SymbolConfig(Symbols* symbols);
	~SymbolConfig();
	
private:
	SymbolConfigListWidget* m_symbolTableView;
	Symbols* m_symbols;
};

class GameConfig : public QWidget, private Ui::ConfigGame {
Q_OBJECT
public:
	explicit GameConfig(QWidget* parent = 0);
};

}

#endif
