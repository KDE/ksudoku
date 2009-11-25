#ifndef GRAPH_INSTANCE_H
#define GRAPH_INSTANCE_H

#include <QVector>
#include <QBitArray>
#include "statearray.h"

#include "storage.h"

class Ruleset;

class Problem {
public:
	Problem();
	Problem(const Ruleset *rules);
	Problem(const Problem& problem);
	~Problem();
	Problem &operator=(const Problem &other);
	// TODO replace this methods by item-based methods
public:
	const Ruleset *ruleset() const { return m_rules; }

	Storage::Instance *storage(const Storage *s) const { return m_storages[s->storageId()]; }
private:
	const Ruleset *m_rules;
	QVector<Storage::Instance*> m_storages;
};

#endif // GRAPH_INSTANCE_H
