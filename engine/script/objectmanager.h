#ifndef _KSUDOKU_OBJECTMANAGER_H_
#define _KSUDOKU_OBJECTMANAGER_H_

#include <QHash>
#include <QVector>

class QScriptEngine;

class Item;
class BoardWrapper;
class ItemWrapper;

typedef Item * (*ItemConstructor)();

class ObjectManager {
public:
	ObjectManager();
	~ObjectManager();
public:
	void addItemType(const QString &name, ItemConstructor ctor);
	BoardWrapper *createBoard();
	ItemWrapper *createItem(const QString &type, QScriptEngine *engine);
private:
	QHash<QString,ItemConstructor> m_itemTypes;
	QVector<BoardWrapper*> m_boardWrappers;
	QVector<ItemWrapper*> m_itemWrappers;
};

#endif // _KSUDOKU_OBJECTMANAGER_H_
