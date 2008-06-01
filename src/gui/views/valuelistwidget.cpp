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

#include "valuelistwidget.h"

#include "valuelistwidget.moc"

#include <QGraphicsSimpleTextItem>
#include <QWheelEvent>

#include <KColorScheme>

namespace ksudoku {
	
SymbolGraphicsItem::SymbolGraphicsItem(const QChar& symbol, QGraphicsItem * parent, QGraphicsScene * scene)
	: QGraphicsItem(parent, scene)
{
	m_text = new QGraphicsSimpleTextItem(symbol, this, scene);

///
    KStatefulBrush stattefulBrush(KColorScheme::View, KColorScheme::NormalText);

    m_text->setBrush(stattefulBrush.brush(QPalette::Active).color());
///

	setSize(10);
}

SymbolGraphicsItem::~SymbolGraphicsItem() {
}

void SymbolGraphicsItem::setSize(double size) {
	m_size = size;
	
	QFont resizedFont = m_text->font();
	resizedFont.setPixelSize(int(size*0.8));
	m_text->setFont(resizedFont);

	QRectF rect = m_text->boundingRect();
	// TODO improve this or replace it
	m_text->setPos(-rect.width()/2, -rect.height()/2 +0.7);
}

void SymbolGraphicsItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {
}

QRectF SymbolGraphicsItem::boundingRect() const {
	return QRectF(-m_size/2, -m_size/2, m_size, m_size);
}




class SymbolSelectionItem : public SymbolGraphicsItem {
public:
	SymbolSelectionItem(const QChar& symbol, int value, ValueListWidget* widget);
	
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	
private:
	ValueListWidget* m_widget;
	int m_value;
};

SymbolSelectionItem::SymbolSelectionItem(const QChar& symbol, int value, ValueListWidget* widget)
	: SymbolGraphicsItem(symbol, 0, widget->scene()), m_widget(widget), m_value(value)
{
}

void SymbolSelectionItem::mousePressEvent(QGraphicsSceneMouseEvent*) {
	m_widget->selectValueItem(m_value);
}




ValueListWidget::ValueListWidget(QWidget* parent)
	: QGraphicsView(parent)
{
	m_table = 0;
	
	setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	
	m_scene = new QGraphicsScene(this);
	setScene(m_scene);
	
	m_selectionItem = new QGraphicsRectItem(-4.5, 0.5, 9, 9, 0, m_scene);

///
    KStatefulBrush stattefulBrush(KColorScheme::View, KColorScheme::NormalText);

    dynamic_cast< QGraphicsRectItem* >(m_selectionItem)->setPen(stattefulBrush.brush(QPalette::Active).color());
///

	m_selectionItem->setPos(0, 0);
	
	m_maxValue = 1;
	m_selectedValue = 1;
}

ValueListWidget::~ValueListWidget() {
}

SymbolTable* ValueListWidget::currentTable() const {
	return m_table;
}

void ValueListWidget::setCurrentTable(SymbolTable* table, int maxValue) {
	m_table = table;
	m_maxValue = maxValue;
	
	SymbolSelectionItem* item;
	while(!m_symbols.empty()) {
		delete m_symbols.takeLast();
	}
	
	if(m_table) {
		for(int i = 0; i < maxValue; ++i) {
			item = new SymbolSelectionItem(m_table->symbolForValue(i+1), i+1, this);
			item->setSize(10);
			item->setPos(0, (i+0.5)*10);
			m_symbols.append(item);
		}
	}
	m_scene->setSceneRect(-5, 0, 10, maxValue*10);
	
	if(m_selectedValue > m_maxValue) m_selectedValue = 1;
	
	m_scene->update();
	
	resizeEvent(0);
}

void ValueListWidget::resizeEvent(QResizeEvent*)
{
	fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void ValueListWidget::selectValue(int value) {
	m_selectedValue = value;
	m_selectionItem->setPos(0, (value-1)*10);
// 	m_scene->update();
}

void ValueListWidget::selectValueItem(int value) {
	selectValue(value);
	emit valueSelected(value);
}

void ValueListWidget::wheelEvent (QWheelEvent* e) {
	int value = (m_selectedValue - e->delta()/120) % m_maxValue;
	if(value <= 0) value = m_maxValue - value;
	selectValueItem(value);
}


}
