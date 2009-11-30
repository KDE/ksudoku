#include "eliminationstorage.h"

#include <QVector>

#include "problem.h"
#include "statearray.h"
#include "ruleset.h"

class EliminationStorage::Instance : public Storage::Instance {
	friend class EliminationStorage;
	friend class EliminationStorage::Entry;
public:
	Storage::Instance *clone() const;
	void clone(const Storage::Instance *other);
private:
	StateArray counts;
};

class EliminationStoragePrivate {
	Q_DECLARE_PUBLIC(EliminationStorage)
	friend class EliminationStorage::Entry;
public:
	EliminationStoragePrivate(EliminationStorage *storage);
protected:
	EliminationStorage *q_ptr;
private:
	int size;
	int states;
	QVector<IVariableItem*> items;
	QVector<int> stateCounts;
};

EliminationStoragePrivate::EliminationStoragePrivate(EliminationStorage *storage) {
	size = 0;
	states = 0;
	q_ptr = storage;
}

EliminationStorage::EliminationStorage() {
	d_ptr = new EliminationStoragePrivate(this);
}

EliminationStorage::~EliminationStorage() {
	delete d_ptr;
}

/**
 * \brief Returns the item with that has the lowest count of possiblities greater than 1
 */
IVariableItem* EliminationStorage::itemWithLeastPossibilitiesLeft(const Problem* inst) const {
	Q_D(const EliminationStorage);
	Instance *storage = static_cast<Instance*>(inst->storage(this));

	int pos = -1;
	for(int i = 2; i <= d->states; ++i) {
		pos = storage->counts.first(i);
		if(pos >= 0) break;
	}

	IVariableItem *item = 0;
	if(pos >= 0) item = d->items[pos];
	return item;
}

bool EliminationStorage::isFinished(const Problem *problem) const {
	Q_D(const EliminationStorage);
	Instance *storage = static_cast<Instance*>(problem->storage(this));
	return storage->counts.count(1) == d->size;
}

void EliminationStorage::reset(Problem* problem) const {
	Q_D(const EliminationStorage);
	Instance *storage = static_cast<Instance*>(problem->storage(this));
	for(int i = 0; i < d->size; ++i) {
		storage->counts.setState(i, d->stateCounts[i]);
	}
}

Storage::Instance *EliminationStorage::create() const {
	Q_D(const EliminationStorage);
	Instance *inst = new Instance();
	inst->counts.resize(d->size, d->states+1, 0);
	for(int i = 0; i < d->size; ++i) {
		inst->counts.setState(i, d->stateCounts[i]);
	}
	return inst;
}

Storage::Instance *EliminationStorage::Instance::clone() const {
	Instance *inst = new Instance();
	inst->counts = counts;
	return inst;
}

void EliminationStorage::Instance::clone(const Storage::Instance *other) {
	const Instance *inst = static_cast<const Instance*>(other);
	counts = inst->counts;
}

EliminationStorage::Entry::Entry(IVariableItem *item, int size) {
	m_storage = 0;
	m_item = item;
	m_index = -1;
	m_possibilities = size;
}

int EliminationStorage::Entry::possibilities() const {
	return m_possibilities;
}

void EliminationStorage::Entry::setPossibilities(int possibilities) {
	m_possibilities = possibilities;
}

IVariableItem *EliminationStorage::Entry::item() const {
	return m_item;
}

void EliminationStorage::Entry::setItem(IVariableItem *item) {
	Q_ASSERT(!m_storage);
	m_item = item;
}

void EliminationStorage::Entry::setup(Ruleset *rules) {
	Q_ASSERT(!m_storage);
	Q_ASSERT(m_index < 0);
	Q_ASSERT(m_item);
	EliminationStorage *metaStorage = storage<EliminationStorage>(rules);
	EliminationStoragePrivate *d = static_cast<EliminationStoragePrivate*>(metaStorage->d_ptr);
	m_index = d->size++;
	d->stateCounts << m_possibilities;
	d->items << m_item;
	if(d->states < m_possibilities) {
		d->states = m_possibilities;
	}
	m_storage = metaStorage;
}

int EliminationStorage::Entry::possibilitiesLeft(const Problem *problem) const {
	Q_ASSERT(m_storage);
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	return storage->counts.state(m_index);
}

void EliminationStorage::Entry::incPossibilitiesLeft(Problem *problem) const {
	Q_ASSERT(m_storage);
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	int count = storage->counts.state(m_index);
	return storage->counts.setState(m_index, count+1);
}

void EliminationStorage::Entry::decPossibilitiesLeft(Problem *problem) const {
	Q_ASSERT(m_storage);
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	int count = storage->counts.state(m_index);
	return storage->counts.setState(m_index, count-1);
}

void EliminationStorage::Entry::setPossibilitiesLeft(Problem *problem, int count) const {
	Q_ASSERT(m_storage);
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	return storage->counts.setState(m_index, count);
}
