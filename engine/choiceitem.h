#ifndef _VALUEITEM_H_
#define _VALUEITEM_H_

#include <QtCore/QtGlobal>

#include <QtCore/QVariant>

#include "item.h"
#include "storage.h"

// TODO rm
#include <QBitArray>
#include "statearray.h"

#include "constraint.h"

#include "eliminationstorage.h"
#include "markerstorage.h"

class ChoiceItem;

class ChoiceStoragePrivate;
class ChoiceStorage : public Storage {
	class Instance;
public:
	class Entry {
		// TODO this friend declaration is only temporary
		friend class ValueMetaItem;
		public:
			Entry();
			void setup(Ruleset *rules);
		public:
			int value(const Problem *problem) const;
			void setValue(Problem *problem, int value) const;
		private:
			ChoiceStorage *m_storage;
			int m_valueIndex;
	};
public:
	ChoiceStorage();
	~ChoiceStorage();
public:
	Storage::Instance *create() const;
	static const char *name() { return "value-items"; }
protected:
	ChoiceStoragePrivate *d_ptr;
private:
	Q_DECLARE_PRIVATE(ChoiceStorage);
};

class ChoiceItem : public Item, public IVariableItem {
	Q_OBJECT
	Q_PROPERTY(int min READ minValue WRITE setMinValue)
	Q_PROPERTY(int max READ maxValue WRITE setMaxValue)
public:
	ChoiceItem();
	ChoiceItem(int minValue, int maxValue);
public:
	static Item *construct(const QVariantList &args);
public:
	int minValue() const { return m_minValue; }
	void setMinValue(int value);
	int maxValue() const { return m_maxValue; }
	void setMaxValue(int value);
public:
	void init(Ruleset *graph);
	
	QDebug debug(QDebug dbg, Problem *problem);
public slots:
	int value(const Problem *problem) const;
	void setValue(Problem *problem, int value) const;
	bool marker(const Problem *problem, int value) const;
	void setMarker(Problem *problem, int value, bool state) const;
public: // implementation of interface VariableItem
	Item *item();
	int possibilities(Problem *problem) const;
	int nextPossibility(Problem *problem, int current) const;
	void applyPossibility(Problem *problem, int variant);
private:
	EliminationStorage::Entry m_eliminator;
	ChoiceStorage::Entry m_value;
	MarkerStorage::Entry m_markers;
	int m_minValue;
	int m_maxValue;
};

// TODO move back to valueitem.cpp
class ChoiceStorage::Instance : public Storage::Instance {
	friend class ChoiceStorage;
	friend class ChoiceStorage::Entry;
public:
	Storage::Instance *clone() const;
	void clone(const Storage::Instance *other);
private:
	QVector<int> m_values;
};

class LastValueInNodeHelper : public ConstraintHelper {
public:
	bool apply(Problem* problem) const;
	Constraint* constraint() const { return m_constraint; }
	ChoiceItem *m_item;
	Constraint* m_constraint;
};

class LastValueInNodeConstraint : public Constraint {
	Q_OBJECT
public:
	LastValueInNodeConstraint(ChoiceItem *item) {
		helper.m_item = item;
		helper.m_constraint = this;
	}
	QVector<ConstraintHelper*> helpers() {
		return QVector<ConstraintHelper*>() << &helper;
	}
	QVector<Item*> affectedItems() const {
		return QVector<Item*>() << helper.m_item;
	}
	void init(Ruleset *rules) {
		helper.setup(rules);
	}
private:
	LastValueInNodeHelper helper;
};

#endif
