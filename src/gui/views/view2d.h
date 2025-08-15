/***************************************************************************
 *   Copyright 2008      Johannes Bergmeier <johannes.bergmeier@gmx.net>   *
 *   Copyright 2015      Ian Wadham <iandw.au@gmail.com>                   *
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
	explicit View2DScene(GameActions* gameActions);
	~View2DScene() override;
public:
	void init(const Game& game);

	/**
	 * Set up the graphics for drawing a Mathdoku or Killer Sudoku cage.
	 * The list of cages goes at the end of the list of rows, columns and
	 * blocks, in the vector QList<GroupGraphicsItem*> m_groups. Each
	 * GroupGraphicsItem is a graphical structure having cells, an outline
	 * or boundary and highlighting. In addition, cages have a cage-label
	 * containing the cage's value and operator.
	 *
	 * @param cageNum      The position of the cage (0 to n) in model-data
	 *                     in the puzzle's SKGraph. Its position in the
	 *                     scene and view list is (offset + cageNum), where
	 *                     "offset" is the number of rows, cols and blocks.
	 * @param drawLabel    Whether the cage-label is to be drawn.
	 */
	void initCageGroup (int cageNum, bool drawLabel = true);
	
	void setSceneSize(const QSize& size);

	void hover(int cell);
	void press(int cell, bool rightButton = false);

	inline int maxValue() const { return m_game.order(); }

public Q_SLOTS:
	void selectValue(int val);
	void enterValue(int val, int cell=-1);
	void markValue(int val, int cell=-1, bool set=true);
	void flipMarkValue(int val, int cell=-1);
	void moveCursor(int dx, int dy);
	void update(int cell = -1);
	void settingsChanged();
	/**
	 * Add, replace or delete graphics for a Mathdoku or Killer Sudoku cage.
	 *
	 * @param cageNumP1    The cage-index (in lists of cages) + 1.
	 *                     Value 1 to N: Add or replace the cage-graphics.
	 *                     Value -1 to -N: delete the cage-graphics.
	 *                     Value 0: invalid.
	 */
	void updateCage (int cageNumP1, bool drawLabel);
Q_SIGNALS:
	void valueSelected(int val);
protected:
	void wheelEvent(QGraphicsSceneWheelEvent* event) override;
private:
	QGraphicsPixmapItem* m_background;
	QGraphicsItem* m_groupLayer;
	QGraphicsItem* m_cellLayer;
	QList<GroupGraphicsItem*> m_groups;
	QList<CellGraphicsItem*> m_cells;
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
	~View2D() override;

public:
	QWidget* widget() override { return this; }
public Q_SLOTS:
	void selectValue(int value) override;
	void settingsChanged();
Q_SIGNALS:
	void valueSelected(int value);
protected:
	void resizeEvent(QResizeEvent* e) override;
private:
	View2DScene* m_scene;
};

}

#endif // _VIEW2D_H_ 
