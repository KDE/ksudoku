#include "markerstorage.h"

#include <QtCore/QBitArray>

#include "ruleset.h"
#include "problem.h"

class MarkerStorage::Instance : public Storage::Instance {
	friend class MarkerStorage;
	friend class MarkerStorage::Entry;
public:
	Storage::Instance *clone() const;
	void clone(const Storage::Instance *other);
private:
	QBitArray data;
};

class MarkerStoragePrivate {
	Q_DECLARE_PUBLIC(MarkerStorage)
	friend class MarkerStorage::Entry;
public:
	MarkerStoragePrivate(MarkerStorage *storage);
protected:
	MarkerStorage *q_ptr;
private:
	int size;
};

// Implementations

MarkerStoragePrivate::MarkerStoragePrivate(MarkerStorage *storage) {
	size = 0;
	q_ptr = storage;
}

MarkerStorage::MarkerStorage() {
	d_ptr = new MarkerStoragePrivate(this);
}

MarkerStorage::~MarkerStorage() {
	delete d_ptr;
}


void MarkerStorage::reset(Problem* problem) const {
	Q_D(const MarkerStorage);
	Instance *storage = static_cast<Instance*>(problem->storage(this));
	storage->data.fill(true, d->size);
}

Storage::Instance *MarkerStorage::create() const {
	Q_D(const MarkerStorage);
	Instance *inst = new Instance();
	inst->data.fill(true, d->size);
	return inst;
}

Storage::Instance *MarkerStorage::Instance::clone() const {
	Instance *inst = new Instance();
	inst->data = data;
	return inst;
}

void MarkerStorage::Instance::clone(const Storage::Instance *other) {
	const Instance *inst = static_cast<const Instance*>(other);
	data = inst->data;
}

MarkerStorage::Entry::Entry(int size) {
	m_index = -1;
	m_size = size;
	m_storage = 0;
}

int MarkerStorage::Entry::size() const {
	return m_size;
}

void MarkerStorage::Entry::setSize(int size) {
	Q_ASSERT(!m_storage);
	Q_ASSERT(m_index < 0);
	m_size = size;
}

void MarkerStorage::Entry::setup(Ruleset *rules) {
	Q_ASSERT(!m_storage);
	Q_ASSERT(m_index < 0);
	MarkerStorage *metaStorage = storage<MarkerStorage>(rules);
	MarkerStoragePrivate *d = static_cast<MarkerStoragePrivate*>(metaStorage->d_ptr);
	m_index = d->size;
	d->size += m_size;
	m_storage = metaStorage;
}

bool MarkerStorage::Entry::marker(const Problem *problem, int index) const {
	Q_ASSERT(m_storage);
	Q_ASSERT(index >= 0 && index < m_size);
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	return storage->data.testBit(m_index + index);
}

void MarkerStorage::Entry::setMarker(Problem *problem, int index, bool state) const {
	Q_ASSERT(m_storage);
	Q_ASSERT(index >= 0 && index < m_size);
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	storage->data.setBit(m_index + index, state);
}
