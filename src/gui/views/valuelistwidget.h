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

#ifndef _KSUDOKUVALUELISTWIDGET_H_
#define _KSUDOKUVALUELISTWIDGET_H_

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QList>

#include "symbols.h"


namespace ksudoku {

class SymbolItem;
class SelectionItem;
	
class ValueListWidget : public QGraphicsView {
	Q_OBJECT
	friend class SymbolItem;
public:
	ValueListWidget(QWidget* parent = 0);
	~ValueListWidget();
	
// 	SymbolTable* currentTable() const;
// 	void setCurrentTable(SymbolTable* table, int maxValue);
	void setMaxValue(int maxValue);
	
	void resizeEvent(QResizeEvent*) override;
	
public slots:
	void selectValue(int value);
	
signals:
	/**
	 * This signal gets emitted when the ValueListWidget itself changed the
	 * selected value. A call of selectValue(int) will not cause an emit.
	 */
	void valueSelected(int value);
	
protected:
	inline QGraphicsScene* scene() { return m_scene; }
	void selectValueItem(int value);
	void wheelEvent (QWheelEvent* e) override;
	
private:
// 	SymbolTable* m_table;
	QGraphicsScene* m_scene;
	QList<SymbolItem*> m_symbols;
	SelectionItem *m_selectionItem;
	int m_selectedValue;
	int m_maxValue;
};

}

#endif
