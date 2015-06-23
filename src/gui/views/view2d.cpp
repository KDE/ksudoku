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

#include "puzzle.h"
#include "gameactions.h"

#include "settings.h"

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
	void setCageLabel (const QString cageLabel);
	
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
	QString m_cageLabel;
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

void CellGraphicsItem::setCageLabel(const QString cageLabel) {
	m_cageLabel = cageLabel;
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
	if (! m_cageLabel.isEmpty()) {
		// TODO - Do this in gui/views/renderer.cpp.
		// TODO - More work on layout of targets, markers and symbols.
		// TODO - Leave everything unchanged for pure Sudoku puzzles.
		// TODO - Do font setup once during resize? Put 0+-x/ in themes?
		// TODO - Draw a background rectangle for the target text?
		int size = pic.width();
		QPainter p(&pic);
		p.setPen(QString("white"));	// TODO - Scribble needs black?
		QFont f = p.font();
		f.setBold(true);
		f.setPixelSize((size+5)/6);
		p.setFont(f);
		p.drawText(size/6, (size+4)/5, m_cageLabel);
		p.end();
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
	GroupGraphicsItem(QVector<QPoint> cells, bool isCage = false);
	~GroupGraphicsItem();
	void hideBlockBorder (bool visible);
public:
	void resize(int gridSize, bool highlight);
	void setHighlight(bool highlight);
	void setHighlight(const QPoint& pos, bool highlight);
private:
	int border(int tl, int tr, int bl, int br, int given);
	void detectType();
	void createContour();
	void createSegment(const QPoint& pos, int shape);
private:
	GroupTypes m_type;
	QVector<QPoint> m_cells;
	QVector<GroupGraphicItemSegment> m_segments;
	bool m_isCage;	// Is the group a cage, as in Killer Sudoku or Mathdoku?
	bool m_borderVisible;
};


GroupGraphicsItem::GroupGraphicsItem(QVector<QPoint> cells, bool isCage) {
	m_cells = cells;
	m_isCage = isCage;
	m_borderVisible = true;
	
	setEnabled(false);
	setAcceptedMouseButtons(0);
	
	detectType();
	if (isCage) {
	    // Draw border around cage, even if it is all in one row or column.
	    // IDW test. m_type = GroupBlock;
	    m_type = GroupCage; // IDW test.
	    setZValue(4); // IDW test.
	}
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

void GroupGraphicsItem::hideBlockBorder (bool hide) {
	if ((m_type == GroupBlock) && hide) {
	    m_borderVisible = false;
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

	// TODO - Does this code *really* affect highlights? See setHighlight().
	// Row and column highlights must go above the GroupBlock boundary-line.
	// IDW test. if(m_type == GroupColumn) setZValue(4);
	// IDW test. else if(m_type == GroupRow) setZValue(4);
	// IDW test. else if(m_type == GroupBlock) setZValue(3);
	//
	// TODO - It really looks better to have the block highlight on top of
	//        the row and column, especially with cages or irregular blocks.
	//        Might also be a good idea to reduce opacity of row and column.
	if(m_type == GroupColumn) setZValue(3);
	else if(m_type == GroupRow) setZValue(3);
	else if(m_type == GroupBlock) setZValue(4);
}

void GroupGraphicsItem::createContour() {
	for(int i = 0; i < m_cells.size(); ++i) {
		int x = m_cells[i].x();
		int y = m_cells[i].y();
		int idx[9];
		// Find neighbours of cell (x, y).
		idx[0] = m_cells.indexOf(QPoint(x-1, y-1));	// North West.
		idx[1] = m_cells.indexOf(QPoint(x,   y-1));	// North.
		idx[2] = m_cells.indexOf(QPoint(x+1, y-1));	// North East.
		idx[3] = m_cells.indexOf(QPoint(x-1, y  ));	// West.
		idx[4] = i;					// Self.
		idx[5] = m_cells.indexOf(QPoint(x+1, y  ));	// East.
		idx[6] = m_cells.indexOf(QPoint(x-1, y+1));	// South West.
		idx[7] = m_cells.indexOf(QPoint(x,   y+1));	// South.
		idx[8] = m_cells.indexOf(QPoint(x+1, y+1));	// South East.
		
		// IDW test. if(idx[1] == -1 && idx[3] == -1 && idx[5] == -1 && idx[7] == -1)
		// IDW test. if(!m_isCage &&	// A cage can consist of one cell and a border.
		if((m_type != GroupCage) &&	// A cage can consist of one cell and a border.
		   idx[1] == -1 && idx[3] == -1 && idx[5] == -1 && idx[7] == -1) // IDW test.
		{
			// No adjoining neighbour to N, S, E or W.
			m_type = GroupSpecial;	// Used in the "X" of XSudoku.
			setZValue(4);		// Same height as row or column.
		}
		
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

	// Row and column groups are drawn only when highlighted.
	// Block groups have a boundary-line, but no middle unless highlighted.
	// Special groups (disjoint cells) have a special color.

	switch(m_type & GroupUnhighlightedMask) {
		case GroupRow:
		case GroupColumn:
			segment.standard = 0;
			break;
		case GroupBlock:
		case GroupCage: // IDW test.
			segment.standard = (shape == 15) ? 0	// No middle.
					   : new QGraphicsPixmapItem(this);
			break;
		default: // Special Group
			segment.standard = new QGraphicsPixmapItem(this);
			break;
	}

	// All groups have highlighting.
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
	if (segment->highlighted) {
	    segment->highlighted->setVisible(highlight);
	    if ((m_type & GroupBlock) == GroupBlock) {
		// Block highlight goes on top of row, column and special
		// highlights.  Block boundary-line goes underneath them.
		// IDW test. setZValue(highlight ? 6 : 3);
		setZValue(highlight ? 8 : 4);
	    }
	    // IDW test. if ((m_type & GroupSpecial) == GroupSpecial) {
	    if (m_type == GroupSpecial) { // IDW test.
		// Special highlight goes on top of unhighlighted special cell.
		setZValue(highlight ? 5 : 4);
	    }
	}
	if(segment->standard) {
	    segment->standard->setVisible(!highlight);
	}
    }
    m_type ^= GroupHighlight;
}

void GroupGraphicsItem::setHighlight(const QPoint& pos, bool highlight) {
	setHighlight(m_cells.contains(pos) && highlight);
}

void GroupGraphicsItem::resize(int gridSize, bool highlight) {
	int size = gridSize*2;
	Renderer* r = Renderer::instance();

	GroupTypes standard = m_type & GroupUnhighlightedMask;
	GroupTypes highlighted = m_type | GroupHighlight;
	
	QVector<GroupGraphicItemSegment>::iterator segment;
	for(segment = m_segments.begin(); segment != m_segments.end(); ++segment) {
		QPointF pos = segment->pos*gridSize;
		// Has standard pixmap item?
		if(m_borderVisible && segment->standard) {
			QPixmap pic = r->renderBorder(segment->shape, standard, size);
			segment->standard->setPixmap(pic);
			segment->standard->setOffset(pos);
		}
		// Highlights on and has highlighted pixmap item?
		if(m_borderVisible && highlight && segment->highlighted) {
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
	m_highlightsOn = false;
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

	// Set the order of the layers.
	m_background = new QGraphicsPixmapItem();
	m_background->setZValue(-7);	// Background.
	addItem(m_background);
	m_groupLayer = new QGraphicsItemGroup();
	m_groupLayer->setZValue(-1);	// Boundary-lines and highlighting.
	addItem(m_groupLayer);
	m_cellLayer = new QGraphicsItemGroup();
	m_cellLayer->setZValue(0);	// Cell outlines and shading.
	m_cellLayer->setHandlesChildEvents(false);
	addItem(m_cellLayer);
	
	SKGraph* g = m_game.puzzle()->graph();
	m_cells.resize(m_game.size());
	m_cursorPos = -1;
	for(int i = 0; i < m_game.size(); ++i) {
		// Do not paint unusable cells (e.g. gaps in Samurai puzzles).
		if (game.value(i) == UNUSABLE) {
			m_cells[i] = 0;
			continue;
		}
		m_cells[i] = new CellGraphicsItem(QPoint(g->cellPosX(i), g->cellPosY(i)), i, this);
		m_cells[i]->setParentItem(m_cellLayer);
		if(game.given(i))
			// TODO - Implement allCellsLookSame preference.
			m_cells[i]->setType(SpecialCellPreset);
			// m_cells[i]->setType(SpecialCell); // IDW test.
		else
			m_cells[i]->setType(SpecialCell);
		if(game.value(i))
			m_cells[i]->setValues(QVector<ColoredValue>() << ColoredValue(game.value(i),0));
		else
			m_cells[i]->setValues(QVector<ColoredValue>());
		if(m_cursorPos < 0) m_cursorPos = i;
	}

	m_groups.resize(g->cliqueCount() + g->cageCount());
	for(int i = 0; i < g->cliqueCount(); ++i) {
		// Set the shape of each group.
		QVector<int> idx = g->clique(i);
		QVector<QPoint> pts = QVector<QPoint>(idx.size());
		for(int j = 0; j < idx.size(); ++j) {
		    pts[j] = QPoint(g->cellPosX(idx[j]), g->cellPosY(idx[j]));
		}
		m_groups[i] = new GroupGraphicsItem(pts);
		m_groups[i]->setParentItem(m_groupLayer);
		// IDW TODO - Draw Killer Sudoku cages inside cell borders?
		// Avoid ugly crossings of cages and blocks in Killer Sudoku.
		// Show borders of blocks only if there are no cages present.
		m_groups[i]->hideBlockBorder((g->cageCount() > 0));
	}
	int offset = g->cliqueCount();
	for(int i = 0; i < g->cageCount(); ++i) {
		// Set the shape of each Mathdoku or KillerSudoku cage.
		QVector<int> idx = g->cage(i);
		QVector<QPoint> pts = QVector<QPoint>(idx.size());
		for(int j = 0; j < idx.size(); ++j) {
		    pts[j] = QPoint(g->cellPosX(idx[j]), g->cellPosY(idx[j]));
		}
		m_groups[offset + i] = new GroupGraphicsItem(pts, true);
		m_groups[offset + i]->setParentItem(m_groupLayer);
		// Set the label of each cage (value and operator), but...
		if (g->cage(i).size() > 1) {		 // Not in single cells.
		    QString str = QString::number(g->cageValue(i));
		    if (g->specificType() == Mathdoku) { // Not in KillerSudoku.
			str = str + QString(" /-x+").mid(g->cageOperator(i), 1);
		    }
		    m_cells[g->cageTopLeft(i)]->setCageLabel(str);
		}
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
	// Fix bug 188162 by ensuring that all markers, as well as cells, are
	// updated and displayed after a Load action.
	update(-1);
}

void View2DScene::setSceneSize(const QSize& size) {
	// Called from View2D::resizeEvent() and View2D::settingsChanged().
	m_highlightsOn = Settings::showHighlights();

	m_background->setPixmap(Renderer::instance()->renderBackground(size));
	
	SKGraph* g = m_game.puzzle()->graph();
	setSceneRect(QRectF(0, 0, size.width(), size.height()));
	
	int width = size.width() / (g->sizeX()+1);
	int height = size.height() / (g->sizeY()+1);
	int grid = qMin(width, height) / 2;
	int offsetX = size.width()/2 - g->sizeX()*grid;
	int offsetY = size.height()/2 - g->sizeY()*grid;

	m_groupLayer->setPos(offsetX, offsetY);
	m_cellLayer->setPos(offsetX, offsetY);
	
	for(int i = 0; i < m_game.size(); ++i) {
		if(m_cells[i] == 0) continue;
		m_cells[i]->resize(grid);
	}
	
	for(int i = 0; i < m_groups.size(); ++i) {
		m_groups[i]->resize(grid, m_highlightsOn);
	}
	
	m_cursor->setPixmap(Renderer::instance()->renderSpecial(SpecialCursor, grid*2));
}

void View2DScene::hover(int cell) {
	m_cursorPos = cell;
// 	qDebug() << "hover cell" << cell << m_cells[cell];
	QPoint pos(m_cells[cell]->pos());
	foreach(GroupGraphicsItem* item, m_groups) {
		item->setHighlight(pos, m_highlightsOn);
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
				// TODO - Implement allCellsLookSame preference.
				m_cells[cell]->setType(SpecialCellPreset);
				// m_cells[cell]->setType(SpecialCell); // IDW test.
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
	SKGraph* g = m_game.puzzle()->graph();
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
