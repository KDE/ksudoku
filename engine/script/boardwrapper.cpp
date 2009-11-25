#include "boardwrapper.h"

#include "item.h"
#include "itemclass.h"
#include "itemwrapper.h"

#include <QDebug>

BoardWrapper::BoardWrapper(ItemBoard* board) {
	m_board = board;
	m_items.resize(board->size(0)*board->size(1)*board->size(2)*board->size(3));
}

BoardWrapper::~BoardWrapper() {
	m_items.clear();
	if(!m_board->parent())
		delete m_board;
}

QScriptValue BoardWrapper::itemAt(int x, int y, int z, int w) {
	return m_items[((w * m_board->size(2) + z) * m_board->size(1) + y) * m_board->size(0) + x];
}

bool BoardWrapper::setItemAt(const QScriptValue& item, int x, int y, int z, int w) {
	ItemWrapper *wrapper = ItemClass::valueToWrapper(item);
	if(!wrapper || !wrapper->item()) return false;
	m_items[((w * m_board->size(2) + z) * m_board->size(1) + y) * m_board->size(0) + x] = item;
	m_board->setItem(wrapper->item(), x, y, z, w);
	return true;
}

void BoardWrapper::init(Ruleset* rules) {
	m_board->init(rules);
}
