#include "choiceitem.h"

#include <QtCore/QVector>
#include <QtCore/QBitArray>
#include "statearray.h"

#include "ruleset.h" // TODO rm
#include "problem.h"

#include <QtCore/QtDebug>

ChoiceItem::ChoiceItem() {
	m_minValue = 0;
	m_maxValue = 0;
	m_eliminator.setItem(this);
	m_eliminator.setPossibilities(1);
	m_markers.setSize(1);
}

ChoiceItem::ChoiceItem(int minValue, int maxValue) {
	m_minValue = minValue;
	m_maxValue = maxValue;
	m_eliminator.setItem(this);
	m_eliminator.setPossibilities(maxValue - minValue + 1);
	m_markers.setSize(maxValue - minValue + 1);
}

Item *ChoiceItem::construct(const QVariantList &args) {
	ChoiceItem *item = 0;
	int min = 0, max = 0;
	if(args.count() >= 2) {
		min = args[0].toInt();
		max = args[1].toInt();
		if(max >= min) {
			item = new ChoiceItem(min, max);
		}
	} else  if(args.count() == 1) {
		max = args[0].toInt();
		if(max >= 0) {
			item = new ChoiceItem(0, max);
		}
	}
	return item;
}

void ChoiceItem::setMinValue(int value) {
	Q_ASSERT(!isInitialized());
	m_minValue = value;
	if(m_maxValue < value) m_maxValue = value;
	m_eliminator.setPossibilities(m_maxValue - m_minValue + 1);
	m_markers.setSize(m_maxValue - m_minValue + 1);
}

void ChoiceItem::setMaxValue(int value) {
	Q_ASSERT(!isInitialized());
	m_maxValue = value;
	if(m_minValue > value) m_minValue = value;
	m_eliminator.setPossibilities(m_maxValue - m_minValue + 1);
	m_markers.setSize(m_maxValue - m_minValue + 1);
}

int ChoiceItem::value(const Problem *problem) const {
	return m_value.value(problem);
}

void ChoiceItem::setValue(Problem *problem, int value) const {
	for(int i = m_minValue; i <= m_maxValue; ++i) {
		setMarker(problem, i, value == i);
	}
	m_value.setValue(problem, value);
	changed(problem);
}

bool ChoiceItem::marker(const Problem *problem, int value) const {
	Q_ASSERT(value >= m_minValue && value <= m_maxValue);
	return m_markers.marker(problem, value - m_minValue);
}

void ChoiceItem::setMarker(Problem *problem, int value, bool state) const {
	Q_ASSERT(value >= m_minValue && value <= m_maxValue);
	// TODO make sure allready set values will be removed
	int index = value - m_minValue;
	if(m_markers.marker(problem, index) != state) {
		m_markers.setMarker(problem, index, state);
		if(state) {
			m_eliminator.incPossibilitiesLeft(problem);
		} else {
			m_eliminator.decPossibilitiesLeft(problem);
		}
		changed(problem);
	}
}

void ChoiceItem::init(Ruleset *rules) {
	Item::init(rules);
	
	Constraint *constraint = new LastValueInNodeConstraint(this);
	// TODO manage constraint localy
	rules->addItem(constraint);
	constraint->init(rules);
	m_eliminator.setup(rules);
	m_value.setup(rules);
	m_markers.setup(rules);
}

QDebug ChoiceItem::debug(QDebug dbg, Problem *problem)
{
	int value = m_value.value(problem);
	dbg.nospace();
	if(value) dbg << value; else dbg << '.';
	dbg << '(';
	for(int i = m_minValue; i <= m_maxValue; ++i) {
		if(m_markers.marker(problem, i - m_minValue))
			dbg << i;
		else
			dbg << ' ';
	}
	dbg << ')';
	return dbg.space();
}

Item *ChoiceItem::item() {
	return this;
}

int ChoiceItem::possibilities(Problem *problem) const {
	return m_eliminator.possibilitiesLeft(problem);
}

int ChoiceItem::nextPossibility(Problem *problem, int current) const {
	for(int i = ((current<m_minValue)?m_minValue:current+1); i <= m_maxValue; ++i) {
		if(marker(problem, i)) {
			return i;
		}
	}
	return -1;
}

void ChoiceItem::applyPossibility(Problem *problem, int variant) {
	setValue(problem, variant);
}

 
Storage::Instance *ChoiceStorage::Instance::clone() const {
	Instance *storage = new Instance();
	storage->m_values = m_values;
	return storage;
}

void ChoiceStorage::Instance::clone(const Storage::Instance *other) {
	const Instance *inst = static_cast<const Instance*>(other);
	m_values = inst->m_values;
// 	m_markerCount = inst->m_markerCount;
}


class ChoiceStoragePrivate {
	Q_DECLARE_PUBLIC(ChoiceStorage);
	friend class ChoiceStorage::Entry;
public:
	ChoiceStoragePrivate(ChoiceStorage *p);
protected:
	ChoiceStorage *q_ptr;
private:
	int itemCount;
};


ChoiceStorage::Entry::Entry() {
	m_valueIndex = -1;
	m_storage = 0;
}

void ChoiceStorage::Entry::setup(Ruleset *rules) {
	Q_ASSERT(!m_storage);
	ChoiceStorage *choiceStorage = storage<ChoiceStorage>(rules);
	ChoiceStoragePrivate *d = static_cast<ChoiceStoragePrivate*>(choiceStorage->d_ptr);
	m_valueIndex = d->itemCount++;
	m_storage = choiceStorage;
}

int ChoiceStorage::Entry::value(const Problem *problem) const {
	Q_ASSERT(m_storage);
	Instance *instance = static_cast<Instance*>(problem->storage(m_storage));
	return instance->m_values[m_valueIndex];
}

void ChoiceStorage::Entry::setValue(Problem *problem, int value) const {
	Q_ASSERT(m_storage);
	Instance *instance = static_cast<Instance*>(problem->storage(m_storage));
	instance->m_values[m_valueIndex] = value;
}

ChoiceStoragePrivate::ChoiceStoragePrivate(ChoiceStorage *q) {
	q_ptr = q;
	itemCount = 0;
}

ChoiceStorage::ChoiceStorage() {
	d_ptr = new ChoiceStoragePrivate(this);
}

ChoiceStorage::~ChoiceStorage() {
	delete d_ptr;
}

Storage::Instance *ChoiceStorage::create() const {
	Q_D(const ChoiceStorage);
	Instance *instance = new Instance();
	instance->m_values.fill(0, d->itemCount);
	return instance;
}

bool LastValueInNodeHelper::apply(Problem* problem) const {
	// Test whether value allready exists
	if(m_item->value(problem)) return true;
	
	int c = 0;
	int val = 0;
	for(int i = m_item->minValue(); i <= m_item->maxValue(); ++i) {
		if(m_item->marker(problem, i)) { ++c; val = i; }
	}
	if(c == 1) {
		m_item->setValue(problem, val);
	} else if(c == 0) {
		return false;
	}
	return true;
}
