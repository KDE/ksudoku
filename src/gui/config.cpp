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

#include "config.h"

#include <KLocale>
#include <QListWidget>
#include <QVBoxLayout>

#include "symbols.h"

#include "config.moc"

namespace ksudoku {
	
SymbolConfigListWidget::SymbolConfigListWidget(const QList<SymbolTable*>& tables, QWidget* parent) : QListWidget(parent) {
	for(int i = 0; i < tables.size(); ++i) {
		SymbolTable* table = tables[i];
		
		QString chars;
		for(int j = 1; j <= table->maxValue(); ++j)
			chars += table->symbolForValue(j) + ' ';
		
		QListWidgetItem* li = new QListWidgetItem(table->text(), this);
		li->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		
		QString tooltip = i18ncp("list arg2 consists of arg1 symbols: arg3", "<html><h4>%2</h4>consists of 1 symbol:<br/>%3</html>", "<html><h4>%2</h4>consists of %1 symbols:<br/>%3</html>", table->maxValue(), table->text(), chars);
		li->setData(Qt::ToolTipRole, tooltip);
		li->setData(Qt::UserRole, table->name());
		
		li->setCheckState(Qt::Unchecked);
	}
}

SymbolConfigListWidget::~SymbolConfigListWidget() {
}

QStringList SymbolConfigListWidget::enabledTables() const {
	QStringList list;
	
	for(int i = 0; i < count(); ++i) {
		QListWidgetItem* li = item(i);
		if(li->checkState() == Qt::Checked)
			list << li->data(Qt::UserRole).toString();
	}
	
	return list;
}

void SymbolConfigListWidget::setEnabledTables(const QStringList& tables) {
	for(int i = 0; i < count(); ++i) {
		QListWidgetItem* li = item(i);
		if(tables.contains(li->data(Qt::UserRole).toString()))
			li->setCheckState(Qt::Checked);
		else
			li->setCheckState(Qt::Unchecked);
	}
}
	

SymbolConfig::SymbolConfig(Symbols* symbols) : m_symbols(symbols) {
	QVBoxLayout* layout = new QVBoxLayout(this);
	m_symbolTableView = new SymbolConfigListWidget(symbols->possibleTables(), this);
	m_symbolTableView->setObjectName("kcfg_Symbols");
	layout->addWidget(m_symbolTableView);
}

SymbolConfig::~SymbolConfig() {
}

GameConfig::GameConfig(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
}

}
