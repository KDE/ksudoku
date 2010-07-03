#include "objectmanager.h"

#include "boardprototype.h"
#include "itemprototype.h"
#include "boardwrapper.h"
#include "itemwrapper.h"

#include "item.h"

Q_DECLARE_METATYPE(Item*)
Q_DECLARE_METATYPE(QVector<Item*>)

ObjectManager::ObjectManager() {
	qRegisterMetaType<Item*>();
	qRegisterMetaType<QVector<Item*> >();
}

ObjectManager::~ObjectManager() {
	qDeleteAll(m_boardWrappers);
	qDeleteAll(m_itemWrappers);
}

void ObjectManager::addItemType(const QString& name, ItemConstructor ctor) {
	m_itemTypes.insert(name, ctor);
}

BoardWrapper* ObjectManager::createBoard() {
	ItemBoard *board = new ItemBoard();

	BoardWrapper *wrapper = new BoardWrapper(board);
	m_boardWrappers.append(wrapper);
	return wrapper;
}

ItemWrapper* ObjectManager::createItem(const QString& type, QScriptEngine *engine) {
	ItemConstructor ctor = m_itemTypes.value(type);
	if(!ctor) return 0;

	Item *item = ctor();
	if(!item) return 0;

	ItemWrapper *wrapper = new ItemWrapper(item, type, engine);
	m_itemWrappers.append(wrapper);
	return wrapper;
}

