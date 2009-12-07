#include "item.h"

#include "problem.h"

#include <QVariant>

#include <QtCore/QDebug>

#include "constraint.h"
#include "ruleset.h"

Item::Item() {
	m_rules = 0;
	m_helperStorage = 0;
	m_hasError = false;
}

Item::~Item() {
}

bool Item::isInitialized() const {
	return m_rules;
}

void Item::init(Ruleset *rules) {
	Q_ASSERT(!m_rules);
	m_rules = rules;
	
	m_helperStorage = storage<ConstraintHelperStorage>(m_rules);
}

QDebug Item::debug(QDebug dbg, Problem *problem) {
	Q_UNUSED(problem);
	return dbg;
}

void Item::addAffectingHelper(ConstraintHelper *helper) {
	m_affected << helper;
}

QVector<const ConstraintHelper *> Item::affectingHelpers() const {
	return m_affected;
}

void Item::changed(Problem* problem) const {
	// Test whether we really need the costly iteration over the helpers
	if(!problem->storage(m_helperStorage)) return;
	
	foreach(const ConstraintHelper *helper, m_affected) {
		helper->itemChanged(problem, this);
	}
}

void Item::setError(const QString &msg) {
	m_hasError = true;
	m_errorMessage = msg;
}

void Item::unsetError() {
	m_hasError = false;
}

bool Item::hasError() const {
	return m_hasError;
}

QString Item::errorMessage() const {
	return m_hasError ? m_errorMessage : QString();
}

ItemList ItemMap::itemsAt(int pos0, int pos1, int pos2, int pos3) const {
	ItemList items;
	int min0 = pos0, min1 = pos1, min2 = pos2, min3 = pos3;
	int max0 = pos0, max1 = pos1, max2 = pos2, max3 = pos3;
	if(min0 < 0) { min0 = 0; max0 = size(0)-1; }
	if(min1 < 0) { min1 = 0; max1 = size(1)-1; }
	if(min2 < 0) { min2 = 0; max2 = size(2)-1; }
	if(min3 < 0) { min3 = 0; max3 = size(3)-1; }
	for(int i3 = min3; i3 <= max3; ++i3) {
		for(int i2 = min2; i2 <= max2; ++i2) {
			for(int i1 = min1; i1 <= max1; ++i1) {
				for(int i0 = min0; i0 <= max0; ++i0) {
					items.append(itemAt(i0, i1, i2, i3));
				}
			}
		}
	}
	return items;
}

QDebug ItemMap::debug(QDebug dbg, Problem *problem) {
	for(int i3 = 0; i3 < size(3); ++i3) {
		if(i3 != 0) dbg.nospace() << "----------------------------------------\n";
		for(int i2 = 0; i2 < size(2); ++i2) {
			if(i2 != 0) dbg.nospace() << "- - - - - - - - - - - - - - - - - - - -\n";
			for(int i1 = 0; i1 < size(1); ++i1) {
				if(i1 != 0) dbg.nospace() << '\n';
				for(int i0 = 0; i0 < size(0); ++i0) {
					itemAt(i0, i1, i2, i3)->debug(dbg, problem);
				}
			}
		}
	}
	
	return dbg.space();
}

ItemBoard::ItemBoard(int size0, int size1, int size2, int size3) {
	m_size[0] = size0; m_size[1] = size1; m_size[2] = size2; m_size[3] = size3;
	m_initialized = false;
}

ItemBoard::~ItemBoard() {
	QVector<Item*>::iterator it;
	for(it = m_items.begin(); it != m_items.end(); ++it) {
		delete *it;
		*it = 0;
	}
}

ItemBoard *ItemBoard::construct(const QVariantList &args) {
	int size0 = 1, size1 = 1, size2 = 1, size3 = 1;

	int c = args.count();
	if(c > 4) c = 4;
	switch(c) {
		case 4: size3 = args[3].canConvert(QVariant::Int) ? args[3].toInt() : 1;
		case 3: size2 = args[2].canConvert(QVariant::Int) ? args[2].toInt() : 1;
		case 2: size1 = args[1].canConvert(QVariant::Int) ? args[1].toInt() : 1;
		case 1: size0 = args[0].canConvert(QVariant::Int) ? args[0].toInt() : 1;
	}
	
	return new ItemBoard(size0, size1, size2, size3);
}

Item *ItemBoard::itemAt(int pos0, int pos1, int pos2, int pos3) const {
	if(!m_initialized) return 0;
	return m_items[index(pos0, pos1, pos2, pos3)];
}

void ItemBoard::setItem(Item *item, int pos0, int pos1, int pos2, int pos3) {
	if(!m_initialized) init();
	int i = index(pos0, pos1, pos2, pos3);
	Q_ASSERT(!m_items[i]);
	m_items[i] = item;
	item->setParent(this);
}

void ItemBoard::init() {
	int size = m_size[0] * m_size[1] * m_size[2] * m_size[3];
	m_items.resize(size);
	m_initialized = true;
}

void ItemBoard::init(Ruleset *rules) {
	if(!m_initialized) init();
	
	QVector<Item*>::iterator it;
	for(it = m_items.begin(); it != m_items.end(); ++it) {
		if(*it) {
			(*it)->init(rules);
		}
	}
}

#include "item.moc"
