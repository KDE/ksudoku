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

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QList>

#include "symbols.h"

class QGraphicsSimpleTextItem;

namespace ksudoku {

class SymbolGraphicsItem : public QGraphicsItem {
public:
	SymbolGraphicsItem(const QChar& symbol,QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);
	~SymbolGraphicsItem();
	
public:
	void setSize(double size);
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
	QRectF boundingRect() const;
	
private:
	QGraphicsSimpleTextItem* m_text;
	double m_size;
};

class SymbolSelectionItem;
	
class ValueListWidget : public QGraphicsView {
friend class SymbolSelectionItem;
Q_OBJECT
public:
	ValueListWidget(QWidget* parent = 0);
	~ValueListWidget();
	
	SymbolTable* currentTable() const;
	void setCurrentTable(SymbolTable* table, int maxValue);
	
	void resizeEvent(QResizeEvent*);
	
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
	void wheelEvent (QWheelEvent* e);
	
private:
	SymbolTable* m_table;
	QGraphicsScene* m_scene;
	QList<SymbolSelectionItem*> m_symbols;
	QGraphicsItem* m_selectionItem;
	int m_selectedValue;
	int m_maxValue;
};

}

#endif
