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
class View2DScene : public QGraphicsScene {
	Q_OBJECT
public:
	View2DScene();
	~View2DScene();
public:
	void init(const Game& game);
	
	void setSceneSize(const QSize& size);

	void hover(int cell);
	void press(int cell);
	void update(int cell = -1);

	void selectValue(int val);

signals:
	void cellHovered(int cell);
private:
	QGraphicsPixmapItem* m_background;
	QGraphicsItem* m_groupLayer;
	QGraphicsItem* m_cellLayer;
	QVector<GroupGraphicsItem*> m_groups;
	QVector<CellGraphicsItem*> m_cells;
	QGraphicsPixmapItem* m_cursor;
	Game m_game;

	int m_selectedValue;
};

class View2D : public QGraphicsView, public ViewInterface {
	Q_OBJECT
public:
	View2D(QWidget *parent, const Game& game);
	~View2D();

public:
	QWidget* widget() { return this; }
public slots:
	void setCursor(int cell) { Q_UNUSED(cell) }
	void selectValue(int value);
	void setSymbols(SymbolTable* table) { Q_UNUSED(table) }
	void setFlags(ViewFlags flags) { Q_UNUSED(flags) }
	void update(int cell = -1);
signals:
	void cellHovered(int cell);
	void valueSelected(int value);
protected:
	virtual void resizeEvent(QResizeEvent* e);
private:
	View2DScene* m_scene;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(ksudoku::GroupTypes)

#endif // _VIEW2D_H_ 
