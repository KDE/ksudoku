#ifndef _KSUDOKU_BOARDWRAPPER_H_
#define _KSUDOKU_BOARDWRAPPER_H_

#include <QVector>
#include <QScriptValue>
#include <QMetaType>

class ItemBoard;
class Ruleset;

class BoardWrapper {
public:
	BoardWrapper(ItemBoard *board);
	~BoardWrapper();
public:
	inline ItemBoard *board() const { return m_board; }
	QScriptValue itemAt(int x, int y, int z, int w);
	bool setItemAt(const QScriptValue &item, int x, int y, int z, int w);
public:
	void init(Ruleset *rules);
private:
	QVector<QScriptValue> m_items;
	ItemBoard *m_board;
};

Q_DECLARE_METATYPE(BoardWrapper*)

#endif // _KSUDOKU_BOARDWRAPPER_H_
