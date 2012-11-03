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

#ifndef _VIEW2D_H_
#define _VIEW2D_H_

#include <QGraphicsView>

#include "ksview.h"

#include "ksudokugame.h"

#include "renderer.h"

class QGraphicsPixmapItem;
namespace ksudoku {

class GroupGraphicsItem;
class CellGraphicsItem;
class GameActions;

class View2DScene : public QGraphicsScene {
	Q_OBJECT
public:
	View2DScene(GameActions* gameActions);
	~View2DScene();
public:
	void init(const Game& game);
	
	void setSceneSize(const QSize& size);

	void hover(int cell);
	void press(int cell, bool rightButton = false);

	inline int maxValue() const { return m_game.order(); }

public slots:
	void selectValue(int val);
	void enterValue(int val, int cell=-1);
	void markValue(int val, int cell=-1, bool set=true);
	void flipMarkValue(int val, int cell=-1);
	void moveCursor(int dx, int dy);
	void update(int cell = -1);
signals:
	void valueSelected(int val);
protected:
	void wheelEvent(QGraphicsSceneWheelEvent* event);
private:
	QGraphicsPixmapItem* m_background;
	QGraphicsItem* m_groupLayer;
	QGraphicsItem* m_cellLayer;
	QVector<GroupGraphicsItem*> m_groups;
	QVector<CellGraphicsItem*> m_cells;
	QGraphicsPixmapItem* m_cursor;
	Game m_game;

	GameActions* m_gameActions;

	int m_cursorPos;
	int m_selectedValue;
	bool m_highlightsOn;
};

class View2D : public QGraphicsView, public ViewInterface {
	Q_OBJECT
public:
	View2D(QWidget* parent, const Game& game, GameActions* gameActions);
	~View2D();

public:
	QWidget* widget() { return this; }
public slots:
	void selectValue(int value);
	void settingsChanged();
signals:
	void valueSelected(int value);
protected:
	virtual void resizeEvent(QResizeEvent* e);
private:
	View2DScene* m_scene;
};

}

#endif // _VIEW2D_H_ 
