/***************************************************************************
 *   Copyright 2008      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
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

#include "view2d.h"

#include "view2d.moc" 

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneEvent>
#include <QtDebug>
// #include <QFile> // TODO only for debug
// #include <QEvent> // TODO only for debug

#include <kdebug.h>

#include "sudoku_solver.h"
#include "puzzle.h"
#include "gameactions.h"

namespace ksudoku {


struct ColoredValue {
	ColoredValue() : value(0), color(0) { }
	ColoredValue(int v, int c) : value(v), color(c) { }
	int value;
	int color;
};

class CellGraphicsItem : public QGraphicsPixmapItem {
public:
	CellGraphicsItem(QPoint pos, int id, View2DScene* scene);
public:
	void resize(int gridSize);
	QPoint pos() const { return m_pos; }
	void showCursor(QGraphicsItem* cursor);
	void setType(SpecialType type);
	
	void setValues(QVector<ColoredValue> values);
protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
private:
	void updatePixmap();
private:
	View2DScene* m_scene;
	QPoint m_pos;
	SpecialType m_type;
	QVector<ColoredValue> m_values;
	int m_id;
	int m_size;
	int m_range;
};

CellGraphicsItem::CellGraphicsItem(QPoint pos, int id, View2DScene* scene) {
	setAcceptsHoverEvents(true);
	setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	m_pos = pos;
	m_size = 0;
	m_scene = scene;
	m_id = id;
	m_type = SpecialCell;
	m_range = scene->maxValue();
}

void CellGraphicsItem::resize(int gridSize) {
	m_size = gridSize * 2;
	
	setPos(m_pos.x()*m_size, m_pos.y()*m_size);
	updatePixmap();
}

void CellGraphicsItem::showCursor(QGraphicsItem* cursor) {
	cursor->setParentItem(this);
	cursor->setZValue(1);
}
	
void CellGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
	Q_UNUSED(event);
	m_scene->hover(m_id);
}

void CellGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	switch(event->button()) {
		case Qt::LeftButton:
			m_scene->press(m_id);
			break;
		case Qt::RightButton:
			m_scene->press(m_id, true);
			break;
		default:
			break;
	}
}

void CellGraphicsItem::setType(SpecialType type) {
	if(type == m_type) return;
	m_type = type;

	updatePixmap();
}

void CellGraphicsItem::setValues(QVector<ColoredValue> values) {
	m_values = values;
	
	updatePixmap();
}

void CellGraphicsItem::updatePixmap() {
	if(m_size == 0) return;

	hide();
	
	QPixmap pic = Renderer::instance()->renderSpecial(m_type, m_size);
	switch(m_type) {
		case SpecialCell:
		case SpecialCellMistake:
			if(m_values.size() > 0) {
				pic = Renderer::instance()->renderSymbolOn(pic, m_values[0].value, m_values[0].color, m_range, SymbolEdited);
			}
			break;
		case SpecialCellPreset:
			if(m_values.size() > 0) {
				pic = Renderer::instance()->renderSymbolOn(pic, m_values[0].value, 0, m_range, SymbolPreset);
			}
			break;
		case SpecialCellMarkers: {
			for(int i = m_values.size()-1; i >= 0; --i) {
				pic = Renderer::instance()->renderMarkerOn(pic, m_values[i].value, m_range, 0);
			}
			} break;
		default: break; // TODO maybe assert as this is not allowed to occur
	}
	setPixmap(pic);
	
	show();
}


struct GroupGraphicItemSegment {
	QPoint pos;
	int shape;
	QGraphicsPixmapItem* standard;
	QGraphicsPixmapItem* highlighted;
};

class GroupGraphicsItem : public QGraphicsItemGroup {
public:
	GroupGraphicsItem(QVector<QPoint> cells);
	~GroupGraphicsItem();
public:
	void resize(int gridSize);
	void setHighlight(bool highlight);
	void setHighlight(const QPoint& pos);
private:
	int border(int tl, int tr, int bl, int br, int given);
	void detectType();
	void createContour();
	void createSegment(const QPoint& pos, int shape);
private:
	GroupTypes m_type;
	QVector<QPoint> m_cells;
	QVector<GroupGraphicItemSegment> m_segments;
};


GroupGraphicsItem::GroupGraphicsItem(QVector<QPoint> cells) {
	m_cells = cells;
	
	setEnabled(false);
	setAcceptedMouseButtons(0);
	
	detectType();
	createContour();

	if(!m_cells.contains(QPoint(1,1))) setHighlight(false);
}

GroupGraphicsItem::~GroupGraphicsItem() {
	QVector<GroupGraphicItemSegment>::iterator segment;
	for(segment = m_segments.begin(); segment != m_segments.end(); ++segment) {
		if(segment->highlighted) delete segment->highlighted;
		if(segment->standard) delete segment->standard;
	}
}

void GroupGraphicsItem::detectType() {
	int x = m_cells[0].x();
	int y = m_cells[0].y();
	for(int i = 1; i < m_cells.size(); ++i) {
		if(x != m_cells[i].x()) x = -1;
		if(y != m_cells[i].y()) y = -1;
	}
	m_type = GroupNone;
	if(x==-1) m_type |= GroupRow;
	if(y==-1) m_type |= GroupColumn;
	
	if(m_type == GroupSpecial) setZValue(0);
	if(m_type == GroupColumn) setZValue(-1);
	else if(m_type == GroupRow) setZValue(-2);
	else if(m_type == GroupBlock) setZValue(-3);
}

void GroupGraphicsItem::createContour() {
	for(int i = 0; i < m_cells.size(); ++i) {
		int x = m_cells[i].x();
		int y = m_cells[i].y();
		int idx[9];
		idx[0] = m_cells.indexOf(QPoint(x-1, y-1));
		idx[1] = m_cells.indexOf(QPoint(x,   y-1));
		idx[2] = m_cells.indexOf(QPoint(x+1, y-1));
		idx[3] = m_cells.indexOf(QPoint(x-1, y  ));
		idx[4] = i;
		idx[5] = m_cells.indexOf(QPoint(x+1, y  ));
		idx[6] = m_cells.indexOf(QPoint(x-1, y+1));
		idx[7] = m_cells.indexOf(QPoint(x,   y+1));
		idx[8] = m_cells.indexOf(QPoint(x+1, y+1));
		
		// TODO FIX: this is a detection outsied of detectType
		if(idx[1] == -1 && idx[3] == -1 && idx[5] == -1 && idx[7] == -1)
			m_type |= GroupSpecial;
		
		int b;
		if((b = border(idx[0],idx[1],idx[3],idx[4],3))) {
			createSegment(QPoint(x,y), b);
		}
		if((b = border(idx[1],idx[2],idx[4],idx[5],2))) {
			createSegment(QPoint(x+1,y), b);
		}
		if((b = border(idx[3],idx[4],idx[6],idx[7],1))) {
			createSegment(QPoint(x,y+1), b);
		}
		if((b = border(idx[4],idx[5],idx[7],idx[8],0))) {
			createSegment(QPoint(x+1,y+1), b);
		}
	}
}

void GroupGraphicsItem::createSegment(const QPoint& pos, int shape) {
	GroupGraphicItemSegment segment;
	segment.pos = pos*2 - QPoint(1,1);
	segment.shape = shape;
	switch(m_type & GroupUnhighlightedMask) {
		// TODO make this behaviour dependant on the availability of normal pixmaps
		case GroupRow:
		case GroupColumn:
			segment.standard = 0;
			break;
		case GroupBlock:
		default: // Special Group
			segment.standard = new QGraphicsPixmapItem(this);
			break;
	}
	segment.highlighted = new QGraphicsPixmapItem(this);
	segment.highlighted->setVisible(false);

	m_segments << segment;
}

int GroupGraphicsItem::border(int tl, int tr, int bl, int br, int given) {
	switch(given) {
		case 0: if(tr > tl || bl > tl ||  br > tl) return 0; break;
		case 1: if(tl > tr || bl > tr ||  br > tr) return 0; break;
		case 2: if(tl > bl || tr > bl ||  br > bl) return 0; break;
		case 3: if(tl > br || tr > br ||  bl > br) return 0; break;
	}
	int b = ((tl!=-1)?1:0) | ((tr!=-1)?2:0) | ((bl!=-1)?4:0) | ((br!=-1)?8:0);
	return b;
}

void GroupGraphicsItem::setHighlight(bool highlight) {
	if(((m_type & GroupHighlight) == GroupHighlight) == highlight) return;
	
	QVector<GroupGraphicItemSegment>::iterator segment;
	for(segment = m_segments.begin(); segment != m_segments.end(); ++segment) {
		if(segment->highlighted) segment->highlighted->setVisible(highlight);
		if(segment->standard) segment->standard->setVisible(!highlight);
	}
	m_type ^= GroupHighlight;
}

void GroupGraphicsItem::setHighlight(const QPoint& pos) {
	setHighlight(m_cells.contains(pos));
}

void GroupGraphicsItem::resize(int gridSize) {
	int size = gridSize*2;
	Renderer* r = Renderer::instance();

	GroupTypes standard = m_type & GroupUnhighlightedMask;
	GroupTypes highlighted = m_type | GroupHighlight;
	
	QVector<GroupGraphicItemSegment>::iterator segment;
	for(segment = m_segments.begin(); segment != m_segments.end(); ++segment) {
		QPointF pos = segment->pos*gridSize;
		if(segment->standard) {
			QPixmap pic = r->renderBorder(segment->shape, standard, size);
			segment->standard->setPixmap(pic);
			segment->standard->setOffset(pos);
		}
		if(segment->highlighted) {
			QPixmap pic = r->renderBorder(segment->shape, highlighted, size);
			segment->highlighted->setPixmap(pic);
			segment->highlighted->setOffset(pos);
		}
	}
}

View2DScene::View2DScene(GameActions* gameActions) {
	m_gameActions = gameActions;
	m_background = 0;
	m_groupLayer = 0;
	m_cellLayer = 0;
	m_cursorPos = 0;
	m_cursor = 0;
}

View2DScene::~View2DScene() {
	delete m_cursor; // needs to be deleted before cells

	// groups need to be deleted before m_groupLayer
	QVector<GroupGraphicsItem*>::iterator group;	
	for(group = m_groups.begin(); group != m_groups.end(); ++group) {
		delete *group;
	}
	
	// cells need to be deleted before m_cellLayer
	QVector<CellGraphicsItem*>::iterator cell;
	for(cell = m_cells.begin(); cell != m_cells.end(); ++cell) {
		delete *cell;
	}
	
	delete m_background;
	delete m_groupLayer;
	delete m_cellLayer;
}

void View2DScene::init(const Game& game) {
	m_selectedValue = 1;
	
	m_game = game;
	
	m_background = new QGraphicsPixmapItem();
	m_background->setZValue(-7);
	addItem(m_background);
	m_groupLayer = new QGraphicsItemGroup();
	m_groupLayer->setZValue(-1);
	addItem(m_groupLayer);
	m_cellLayer = new QGraphicsItemGroup();
	m_cellLayer->setHandlesChildEvents(false);
	addItem(m_cellLayer);
	
	Graph* g = m_game.puzzle()->solver()->g;
	m_cells.resize(m_game.size());
	for(int i = 0; i < m_game.size(); ++i) {
		bool notConnectedNode = ((GraphCustom*) m_game.puzzle()->solver()->g)->optimized_d[i] == 0;			
		if(notConnectedNode) {
			m_cells[i] = 0;
			continue;
		}
		m_cells[i] = new CellGraphicsItem(QPoint(g->cellPosX(i), g->cellPosY(i)), i, this);
		m_cells[i]->setParentItem(m_cellLayer);
		if(game.given(i))
			m_cells[i]->setType(SpecialCellPreset);
		else
			m_cells[i]->setType(SpecialCell);
		if(game.value(i))
			m_cells[i]->setValues(QVector<ColoredValue>() << ColoredValue(game.value(i),0));
		else
			m_cells[i]->setValues(QVector<ColoredValue>());
	}
	
	Graph2d* g2 = dynamic_cast<Graph2d*>(g);
	m_groups.resize(g2->cliqueCount());
	for(int i = 0; i < g2->cliqueCount(); ++i) {
		QVector<int> idx = g2->clique(i);
		QVector<QPoint> pts = QVector<QPoint>(idx.size());
		for(int j = 0; j < idx.size(); ++j) {
			pts[j] = QPoint(g2->cellPosX(idx[j]), g2->cellPosY(idx[j]));
		}
		m_groups[i] = new GroupGraphicsItem(pts);
		m_groups[i]->setParentItem(m_groupLayer);
	}
	
	m_cursor = new QGraphicsPixmapItem();
	addItem(m_cursor);
	
	hover(m_cursorPos);
	
	connect(m_game.interface(), SIGNAL(cellChange(int)), this, SLOT(update(int)));
	connect(m_game.interface(), SIGNAL(fullChange()), this, SLOT(update()));
	connect(m_gameActions, SIGNAL(selectValue(int)), this, SLOT(selectValue(int)));
	connect(m_gameActions, SIGNAL(enterValue(int)), this, SLOT(enterValue(int)));
	connect(m_gameActions, SIGNAL(markValue(int)), this, SLOT(flipMarkValue(int)));
	connect(m_gameActions, SIGNAL(move(int,int)), this, SLOT(moveCursor(int,int)));
}

void View2DScene::setSceneSize(const QSize& size) {
	m_background->setPixmap(Renderer::instance()->renderBackground(size));
	
	Graph* g = m_game.puzzle()->solver()->g;
	setSceneRect(QRectF(0, 0, size.width(), size.height()));
	
	int width = size.width() / (g->sizeX()+1);
	int height = size.height() / (g->sizeY()+1);
	int grid = ((width < height) ? width : height) / 2;
	int offsetX = size.width()/2 - g->sizeX()*grid;
	int offsetY = size.height()/2 - g->sizeY()*grid;

	m_groupLayer->setPos(offsetX, offsetY);
	m_cellLayer->setPos(offsetX, offsetY);
	
	for(int i = 0; i < m_game.size(); ++i) {
		if(m_cells[i] == 0) continue;
		m_cells[i]->resize(grid);
	}
	
	for(int i = 0; i < m_groups.size(); ++i) {
		m_groups[i]->resize(grid);
	}
	
	m_cursor->setPixmap(Renderer::instance()->renderSpecial(SpecialCursor, grid*2));
}

void View2DScene::hover(int cell) {
	m_cursorPos = cell;
	QPoint pos(m_cells[cell]->pos());
	foreach(GroupGraphicsItem* item, m_groups) {
		item->setHighlight(pos);
	}
	
	m_cells[cell]->showCursor(m_cursor);
}

void View2DScene::press(int cell, bool rightButton) {
	if(rightButton) {
		m_game.flipMarker(cell, m_selectedValue);
	} else {
		if(m_game.given(cell)) {
                        selectValue(m_game.value(cell));
		} else {
			m_game.setValue(cell, m_selectedValue);
		}
	}
	
}

void View2DScene::update(int cell) {
	if(cell < 0) {
		for(int i = 0; i < m_game.size(); ++i) {
			if(m_cells[i] == 0) continue;
			update(i);
		}
	} else {
		if(m_cells[cell] == 0) return;
		CellInfo cellInfo = m_game.cellInfo(cell);

		QVector<ColoredValue> values;
		switch(cellInfo.state()) {
			case GivenValue:
				m_cells[cell]->setType(SpecialCellPreset);
				if(cellInfo.value()) values << ColoredValue(cellInfo.value(),0);
				m_cells[cell]->setValues(values);
				break;
			case CorrectValue:
				m_cells[cell]->setType(SpecialCell);
				if(cellInfo.value()) values << ColoredValue(cellInfo.value(),0);
				m_cells[cell]->setValues(values);
				break;
			case WrongValue:
			case ObviouslyWrong:
				m_cells[cell]->setType(SpecialCellMistake);
				if(cellInfo.value()) values << ColoredValue(cellInfo.value(),0);
				m_cells[cell]->setValues(values);
				break;
			case Marker: {
				m_cells[cell]->setType(SpecialCellMarkers);
				for(int i = 1; i <= m_game.size(); ++i) {
					if(cellInfo.marker(i))
						values << ColoredValue(i,0);
				}
				m_cells[cell]->setValues(values);
			} break;
		}
}
}

void View2DScene::selectValue(int value) {
 	m_selectedValue = value;
 	emit valueSelected( value );
}

void View2DScene::enterValue(int value, int cell) {
	if(value >= 0) {
		if(cell >= 0) {
			m_game.setValue(cell, value);
		} else {
			m_game.setValue(m_cursorPos, value);
		}
	} else {
		if(cell >= 0) {
			m_game.setValue(cell, m_selectedValue);
		} else {
			m_game.setValue(m_cursorPos, m_selectedValue);
		}
	}
}

void View2DScene::markValue(int value, int cell, bool set) {
	if(value >= 0) {
		if(cell >= 0) {
			m_game.setMarker(cell, value, set);
		} else {
			m_game.setMarker(m_cursorPos, value, set);
		}
	} else {
		if(cell >= 0) {
			m_game.setMarker(cell, m_selectedValue, set);
		} else {
			m_game.setMarker(m_cursorPos, m_selectedValue, set);
		}
	}
}

void View2DScene::flipMarkValue(int value, int cell) {
	if(value >= 0) {
		if(cell >= 0) {
			m_game.flipMarker(cell, value);
		} else {
			m_game.flipMarker(m_cursorPos, value);
		}
	} else {
		if(cell >= 0) {
			m_game.flipMarker(cell, m_selectedValue);
		} else {
			m_game.flipMarker(m_cursorPos, m_selectedValue);
		}
	}
}

void View2DScene::moveCursor(int dx, int dy) {
	Graph* g = m_game.puzzle()->solver()->g;
	QPoint oldPos = m_cells[m_cursorPos]->pos();
	QPoint relPos;
	int newCursorPos = -1;
	if(dx < 0) relPos.setX(-1);
	else if(dx > 0) relPos.setX(+1);
	if(dy < 0) relPos.setY(-1);
	else if(dy > 0) relPos.setY(+1);
	
	QPoint newPos = oldPos + relPos;
	while(newPos != oldPos && newCursorPos == -1) { 
		if(newPos.x() < 0) newPos.setX(g->sizeX()-1);
		if(newPos.x() >= g->sizeX()) newPos.setX(0);
		if(newPos.y() < 0) newPos.setY(g->sizeY()-1);
		if(newPos.y() >= g->sizeY()) newPos.setY(0);
		
		for(int i = 0; i < m_game.size(); ++i) {
			if(m_cells[i] == 0) continue;
			if(m_cells[i]->pos() == newPos) {
				newCursorPos = i;
				break;
			}
		}
		
		newPos += relPos;
	}
	if(newCursorPos >= 0)
		hover(newCursorPos);
}

void View2DScene::wheelEvent(QGraphicsSceneWheelEvent* event) {
	if(event->orientation() != Qt::Vertical) return;
	
	if(event->delta() < 0) {
		m_selectedValue++;
		if(m_selectedValue > m_game.order())
			m_selectedValue = 1;
	} else if(event->delta() > 0) {
		m_selectedValue--;
		if(m_selectedValue < 1)
			m_selectedValue = m_game.order();
	}
	emit valueSelected(m_selectedValue);
}


View2D::View2D(QWidget *parent, const Game& game, GameActions* gameActions) : QGraphicsView(parent) {
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameStyle(QFrame::NoFrame);
	setAlignment( Qt::AlignLeft | Qt::AlignTop );
	
	m_scene = new View2DScene(gameActions);
	m_scene->init(game);
	setScene(m_scene);

	gameActions->associateWidget(this);
	
	connect(m_scene, SIGNAL(valueSelected(int)), this, SIGNAL(valueSelected(int)));
}

View2D::~View2D() {
	delete m_scene;
}

void View2D::resizeEvent(QResizeEvent* e) {
	if(e) QGraphicsView::resizeEvent(e);
	
	if(m_scene) m_scene->setSceneSize(size());
}

void View2D::selectValue(int value) {
	if(!m_scene) return;
	
	m_scene->selectValue(value);
}

void View2D::settingsChanged() {
	m_scene->setSceneSize(size());
}

}
