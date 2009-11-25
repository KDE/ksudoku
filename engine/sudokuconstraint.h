#ifndef _KSUDOKU_SUDOKUCONSTRAINT_H_
#define _KSUDOKU_SUDOKUCONSTRAINT_H_

#include "constraint.h"

class ChoiceItem;
class SudokuConstraint : public Constraint {
	Q_OBJECT
	Q_PROPERTY(QVector<Item*> items READ affectedItems WRITE setItems)
public:
	Q_INVOKABLE SudokuConstraint();
public:
	QVector<ConstraintHelper*> helpers();
	QVector<Item*> affectedItems() const;
	void setItems(const QVector<Item*> &items);
	Q_INVOKABLE bool addItem(Item *item);

	void init(Ruleset *rules);
private:
	class ClearGroupHelper : public ConstraintHelper {
	public:
		bool apply(Problem* problem) const;
		Constraint* constraint() const;
		SudokuConstraint* m_constraint;
	};
	class LastInGroupHelper : public ConstraintHelper {
	public:
		bool apply(Problem* problem) const;
		Constraint* constraint() const;
		SudokuConstraint* m_constraint;
	};
	QVector<ChoiceItem*> m_items;
	bool m_initialized;
	LastInGroupHelper m_lastInGroupHelper;
	ClearGroupHelper m_clearGroupHelper;
	int m_minValue;
	int m_maxValue;
};

#endif
