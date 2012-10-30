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

#include "renderer.h"

namespace ksudoku {

class SymbolItem : public QGraphicsPixmapItem {
public:
	SymbolItem(int value, int maxValue, ValueListWidget* widget);
public:
	int value() const;
	int maxValue() const;
	void setSize(double size);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
private:
	int m_value;
	int m_maxValue;
	double m_size;
	ValueListWidget* m_widget;
};


SymbolItem::SymbolItem(int value, int maxValue, ValueListWidget* widget)
	: m_widget(widget)
{
	setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	m_value = value;
	m_maxValue = maxValue;
}

int SymbolItem::value() const {
	return m_value;
}

void SymbolItem::setSize(double size) {
	QPixmap pic = Renderer::instance()->renderSpecial(SpecialListItem, size);
	pic = Renderer::instance()->renderSymbolOn(pic, m_value, 0, m_maxValue, SymbolPreset);

	hide();
	setPixmap(pic);
	setOffset(-size*0.5, -size*0.5);
	setPos(0, (m_value-1)*size);
	show();
}

void SymbolItem::mousePressEvent(QGraphicsSceneMouseEvent*) {
	m_widget->selectValueItem(value());
}



class SelectionItem : public QGraphicsPixmapItem {
public:
	SelectionItem();
public:
	void setSize(double size);
	int selectedValue() const;
	void selectValue(int value);
public:
	double m_size;
	int m_value;
};

SelectionItem::SelectionItem() {
	m_value = 0;
	setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
}

void SelectionItem::setSize(double size) {
	QPixmap pic = Renderer::instance()->renderSpecial(SpecialListCursor, size);
	m_size = size;

	hide();
	setPixmap(pic);
	setOffset(-size*0.5, -size*0.5);
	setPos(0, (m_value-1) * m_size);
	show();
}

int SelectionItem::selectedValue() const {
	return m_value;
}

void SelectionItem::selectValue(int value) {
	m_value = value;
	setPos(0, (m_value-1) * m_size);
}


ValueListWidget::ValueListWidget(QWidget* parent)
	: QGraphicsView(parent)
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	
	m_scene = new QGraphicsScene(this);
	setScene(m_scene);
	
	m_selectionItem = new SelectionItem();
	m_scene->addItem(m_selectionItem);
	
	setMaxValue(1);	// Make SymbolItem list non-empty, in case of a resize.
	m_selectedValue = 1;
}

ValueListWidget::~ValueListWidget() {
}

void ValueListWidget::setMaxValue(int maxValue) {
	m_maxValue = maxValue;

	SymbolItem* item;
	QList<SymbolItem*>::iterator it;
	for(it = m_symbols.begin(); it != m_symbols.end(); ++it) {
		delete *it;
	}
	m_symbols.clear();
	
	for(int i = 0; i < maxValue; ++i) {
		item = new SymbolItem(i+1, maxValue, this);
		item->setSize(20);
		item->setPos(0, (i+0.5)*20);
		m_scene->addItem(item);
		m_symbols.append(item);
	}
	
	if(m_selectedValue > m_maxValue) m_selectedValue = 1;

	resizeEvent(0);
}

void ValueListWidget::resizeEvent(QResizeEvent*)
{
	QSize s = size();
	// Allow 4 as a margin and to avoid problems with the border.
	int size = qMin((s.width() - 4), (s.height() - 4)/m_maxValue);
	size = (size < 4) ? 4 : size;	// NB. Negative size --> painter errors.
	for(int i = 0; i < m_maxValue; ++i) {
		m_symbols[i]->setSize(size);
	}
	m_scene->setSceneRect(-size/2, -size/2, size, m_maxValue*size);

	m_selectionItem->setSize(size);
}

void ValueListWidget::selectValue(int value) {
	m_selectedValue = value;
	m_selectionItem->selectValue(value);
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
