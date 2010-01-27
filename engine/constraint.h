#ifndef _KSUDOKU_CONSTRAINT_H_
#define _KSUDOKU_CONSTRAINT_H_

#include <QVector>

#include "item.h"
#include "constrainthelperstorage.h"

class Ruleset;
class Constraint;
class Problem;
class ConstraintHelper;

class ConstraintHelper {
public:
	ConstraintHelper();
	virtual ~ConstraintHelper() {}
public:
	virtual Constraint* constraint() const = 0;
	void setup(Ruleset *ruleset);
    void itemChanged(Problem* problem, const Item* item) const;
	bool resolve(Problem *problem) const;
	bool isPending(Problem *problem) const;
protected:
	virtual bool apply(Problem* problem) const = 0;
public:
	int index;
private:
	ConstraintHelperStorage::Entry m_entry;
};

class Ruleset;
class Constraint : public Item {
	Q_OBJECT
public:
	Constraint();
	virtual ~Constraint() {}
	virtual QVector<ConstraintHelper*> helpers() = 0;

	virtual QVector<Item*> affectedItems() const = 0;
	virtual bool reallyAffectsItem(const Item *item, Problem *puzzle) const;

	virtual void init(Ruleset *rules);
};

#endif
