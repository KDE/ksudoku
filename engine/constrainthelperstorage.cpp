#include "constrainthelperstorage.h"

#include <QVector>

#include "statearray.h"
#include "ruleset.h"
#include "solver.h"

enum HelperState {
	HelperIdle = 0,
	HelperActive = 1
};

class ConstraintHelperStorage::Instance : public Storage::Instance {
	friend class ConstraintHelperStorage;
	friend class ConstraintHelperStorage::Entry;
public:
	Storage::Instance *clone() const;
	void clone(const Storage::Instance *other);
private:
	StateArray states;
};

class ConstraintHelperStoragePrivate {
	Q_DECLARE_PUBLIC(ConstraintHelperStorage)
public:
	ConstraintHelperStoragePrivate(ConstraintHelperStorage *storage);
protected:
	ConstraintHelperStorage *q_ptr;
private:
	QVector<ConstraintHelper *> helpers;
};

ConstraintHelperStoragePrivate::ConstraintHelperStoragePrivate(ConstraintHelperStorage* storage) {
	q_ptr = storage;
}


ConstraintHelperStorage::ConstraintHelperStorage() {
	d_ptr = new ConstraintHelperStoragePrivate(this);
}


ConstraintHelperStorage::~ConstraintHelperStorage() {
	delete d_ptr;
}


ConstraintHelper* ConstraintHelperStorage::firstActiveHelper(Problem* problem) const {
	Q_D(const ConstraintHelperStorage);
	Instance *storage = static_cast<Instance*>(problem->storage(this));
	if(!storage->states.count(HelperActive))
		return 0;
	
	return d->helpers[storage->states.first(HelperActive)];
}


int ConstraintHelperStorage::activeHelpers(Problem* problem) const {
	Instance *storage = static_cast<Instance*>(problem->storage(this));
	return storage->states.count(HelperActive);
}

void ConstraintHelperStorage::reset(Problem* problem) const {
	Instance *storage = static_cast<Instance*>(problem->storage(this));
	storage->states.reset(HelperActive);
}

Storage::Instance* ConstraintHelperStorage::create() const {
	Q_D(const ConstraintHelperStorage);
	Instance *inst = new Instance();
	inst->states.resize(d->helpers.size(), 2, HelperActive);
	return inst;
}

Storage::Instance* ConstraintHelperStorage::Instance::clone() const {
	Instance *inst = new Instance();
	inst->states = states;
	return inst;
}


void ConstraintHelperStorage::Instance::clone(const Storage::Instance* other) {
	const Instance *inst = static_cast<const Instance*>(other);
	states = inst->states;
}


ConstraintHelperStorage::Entry::Entry(ConstraintHelper* helper) {
	m_storage = 0;
	m_helper = helper;
	m_index = -1;
}

void ConstraintHelperStorage::Entry::setup(Ruleset* rules) {
	Q_ASSERT(!m_storage);
	Q_ASSERT(m_index < 0);
	Q_ASSERT(m_helper);
	
	ConstraintHelperStorage *stor = storage<ConstraintHelperStorage>(rules);
	ConstraintHelperStoragePrivate *d = static_cast<ConstraintHelperStoragePrivate*>(stor->d_ptr);
	m_index = d->helpers.size();
	d->helpers << m_helper;
	m_storage = stor;
}

void ConstraintHelperStorage::Entry::activate(Problem* problem) const {
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	storage->states.setState(m_index, HelperActive);
}

void ConstraintHelperStorage::Entry::resolve(Problem* problem) const {
	Instance *storage = static_cast<Instance*>(problem->storage(m_storage));
	storage->states.setState(m_index, HelperIdle);
}

