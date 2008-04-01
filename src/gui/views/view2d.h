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
	void press(int cell);

public slots:
	void selectValue(int val);
	void enterValue(int val, int cell=-1);
	void moveCursor(int dx, int dy);
	void update(int cell = -1);

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
