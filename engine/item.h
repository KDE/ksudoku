#ifndef _PUZZLETYPEDOM_H_
#define _PUZZLETYPEDOM_H_

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>

#include <QtCore/QVariant>

class Ruleset;
class Problem;
class ConstraintHelper;
class ConstraintHelperStorage;

class Item : public QObject {
	Q_OBJECT
public:
	Item();
	virtual ~Item();
public:
	bool isInitialized() const;
	virtual void init(Ruleset *rules);
	virtual QDebug debug(QDebug dbg, Problem *problem) const;
	void addAffectingHelper(ConstraintHelper *helper);
	QVector<const ConstraintHelper *> affectingHelpers() const;

	void changed(Problem *problem) const;
public:
	void setError(const QString &msg = QString());
	void unsetError();
	bool hasError() const;
	QString errorMessage() const;
private:
	QVector<const ConstraintHelper *> m_affected;
	ConstraintHelperStorage *m_helperStorage;
	bool m_hasError;
	QString m_errorMessage;
	Ruleset *m_rules;
};

// interface
class IVariableItem {
public:
	virtual ~IVariableItem() {}
public:
	// Returns the item
	virtual Item *item() = 0;
	// Returns the count of possibilities
	virtual int possibilities(Problem *problem) const = 0;
	// Returns the next variant after variant i, i = -1 => first variant, res = -1 => last variant
	virtual int nextPossibility(Problem *problem, int current = -1) const = 0;
	// Applies the possiblity variant
	virtual void applyPossibility(Problem *problem, int variant) = 0;
};

class ItemList : public QVector<Item*> {
};

class ItemMap : public Item {
	Q_OBJECT
public:
public:
	QString name() const;
	void setName(const QString &) const;
	bool isInitialized() const { return false; }

public:
	Q_INVOKABLE virtual Item *itemAt(int pos0 = 0, int pos1 = 0, int pos2 = 0, int pos3 = 0) const = 0;
	Q_INVOKABLE virtual ItemList itemsAt(int pos0 = -1, int pos1 = -1, int pos2 = -1, int pos3 = -1) const;
	Q_INVOKABLE virtual void setItem(Item *item, int pos0 = 0, int pos1 = 0, int pos2 = 0, int pos3 = 0) = 0;
	Q_INVOKABLE virtual int size(int dim) const = 0;
public:
	virtual QDebug debug(QDebug dbg, Problem *problem) const;
public:
	QString m_name;
};

class ItemBoard : public ItemMap {
	Q_OBJECT
public:
	explicit Q_INVOKABLE ItemBoard(int size0 = 1, int size1 = 1, int size2 = 1, int size3 = 1);
	~ItemBoard();
public:
	static ItemBoard *construct(const QVariantList &args);
public:
	Item *itemAt(int pos0 = 0, int pos1 = 0, int pos2 = 0, int pos3 = 0) const;
	void setItem(Item *item, int pos0 = 0, int pos1 = 0, int pos2 = 0, int pos3 = 0);
	inline int size(int dim) const;
	void init(Ruleset *rules);

	inline int index(int pos0 = 0, int pos1 = 0, int pos2 = 0, int pos3 = 0) const;
private:
	void init();
private:
	bool m_initialized;
	QVector<Item*> m_items;
	int m_size[4];
};

inline int ItemBoard::index(int pos0, int pos1, int pos2, int pos3) const {
	Q_ASSERT(pos0 < m_size[0] && pos1 < m_size[1] && pos2 < m_size[2] && pos3 < m_size[3]);
	return pos0 + m_size[0] * (pos1 + m_size[1] * (pos2 + m_size[2] * pos3));
}

inline int ItemBoard::size(int dim) const {
	return m_size[dim];
}

#endif 
