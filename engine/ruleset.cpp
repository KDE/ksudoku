#include "ruleset.h" 

#include <iostream>

#include "constraint.h"
#include "problem.h"

/**
 * \class Graph
 *
 * \brief Graph represents the topology and the rules of a puzzle.
 *
 */

/**
 * \fn int Graph::valueCount() const
 *
 * Returns the maximum of different values.
 */

/**
 * \fn int Graph::maxValue(int index) const
 *
 * Returns the maximum value for cell \a index.
 * Currently same as valueCount()
 */

/**
 * \fn int Graph::sizeX() const
 *
 * Returns the width of the game.
 */

/**
 * \fn int Graph::sizeY() const
 *
 * Returns the height of the game.
 */

/**
 * \fn int Graph::index(int x, int y) const
 *
 * Returns the index of the cell at (\a x, \a y).
 */

/**
 * \fn int Graph::size() const
 *
 * Returns the count of cells in the gamefield.
 */

/**
 * \fn int Graph::markerStart(int index) const
 *
 * Returns the position of cell \a index relative to the internal cache.
 */

/**
 * \fn QVector<const ConstraintHelper*>& helpers() const
 *
 * Returns the list of helpers.
 */

/**
 * \fn QVector<int> affectedHelpers(int node) const
 *
 * Returns the list of helpers that need to be (re)checked after
 * changing \a node.
 */

/**
 * Constructs a new graph with the width \a sizeX, height \a sizeY,
 * and support for \a values different values at maximum.
 */
Ruleset::Ruleset() {
}

/**
 * Destructs the graph.
 */
Ruleset::~Ruleset() {
	QVector<Item*>::iterator it;
	for(it = m_items.begin(); it != m_items.end(); ++it) {
		delete *it;
	}
}

/**
 * \internal
 */
void Ruleset::addHelper(ConstraintHelper* helper) {
	int pos = m_helpers.size();
	m_helpers << helper;

	foreach(Item *item, helper->constraint()->affectedItems()) {
		item->addAffectingHelper(helper);
		helper->index = pos;
	}
}

/**
 * Adds \a item to the graph
 *
 * \a item is owned by graph and will be deleted at destruction.
 */
void Ruleset::addItem(Item *item) {
	item->setParent(this);
	m_items << item;
}

QVector<Storage*> Ruleset::storages() const {
	return m_storages;
}

Storage *Ruleset::storage(int id) {
	return m_storages[id];
}

const Storage *Ruleset::storage(int id) const {
	return m_storages[id];
}

int Ruleset::storageId(const QByteArray &name) const {
	return m_storageIds.value(name, -1);
}

int Ruleset::regStorage(const QByteArray &name, Storage *storage) {
	Q_ASSERT(!m_storageIds.contains(name));
	Q_ASSERT(storage->storageId() < 0);
	int id = m_storages.count();
	m_storages << storage;
	m_storageIds.insert(name, id);
	storage->setStorageId(id);
	return id;
}

#include "ruleset.moc"
