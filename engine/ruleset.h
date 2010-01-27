#ifndef _KSUDOKU_GRAPH_H_
#define _KSUDOKU_GRAPH_H_

#include <QObject>
#include <QVector>
#include <QHash>

class Constraint;
class ConstraintHelper;
class Item;
class Storage;

class Ruleset : public QObject {
	Q_OBJECT
	friend class Constraint;
	friend class ConstraintHelper;
public:
	Ruleset();
	virtual ~Ruleset();
public:
	
public:
	const QVector<const ConstraintHelper*>& helpers() const {
		return m_helpers;
	}

	QVector<Storage*> storages() const;
	Storage *storage(int id);
	const Storage *storage(int id) const;
	int storageId(const QByteArray &name) const;
	int regStorage(const QByteArray &name, Storage *storage);

	void addItem(Item *item);
	QVector<Item*> itemList() const { return m_items; }
private:
	void addHelper(ConstraintHelper* helper);

private:
	QVector<const ConstraintHelper*> m_helpers;
	QVector<Item*> m_items;
	QHash<QByteArray, int> m_storageIds;
	QVector<Storage*> m_storages;
};

template<class T>
T *storage(Ruleset *rules) {
	int storageId = rules->storageId(T::name());
	if(storageId >= 0) {
		return static_cast<T*>(rules->storage(storageId));
	}

	T *storage = new T();
	rules->regStorage(T::name(), storage);
	return storage;
}

template<class T>
T *storage(const Ruleset *rules) {
	int storageId = rules->storageId(T::name());
	if(storageId >= 0) {
		return static_cast<T*>(rules->storage(storageId));
	}
	return 0;
}

#endif
