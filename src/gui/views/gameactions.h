#ifndef _KSUDOKU_GAMEACTIONS_H_
#define _KSUDOKU_GAMEACTIONS_H_

#include <QVector>
#include <QObject>

class KAction;
class KActionCollection;
class QSignalMapper;

namespace ksudoku {

class GameActions : public QObject {
	Q_OBJECT
public:
	GameActions(KActionCollection* collection);
	void init();
	void associateWidget(QWidget* widget);
signals:
	void selectValue(int value);
	void enterValue(int value = -1);
	void markValue(int value = -1);
	void move(int dx, int dy);
private slots:
	void clearValue();
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
private:
	KActionCollection* m_collection;
	QSignalMapper* m_selectValueMapper;
	QSignalMapper* m_enterValueMapper;
	QSignalMapper* m_markValueMapper;
	QVector<KAction*> m_actions;
};

}

#endif
