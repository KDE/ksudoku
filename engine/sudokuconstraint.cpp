#include "sudokuconstraint.h"

#include "choiceitem.h"

SudokuConstraint::SudokuConstraint() {
	m_initialized = false;
	m_lastInGroupHelper.m_constraint = this;
	m_clearGroupHelper.m_constraint = this;
}

QVector<ConstraintHelper*> SudokuConstraint::helpers() {
	return QVector<ConstraintHelper*>() << &m_lastInGroupHelper << &m_clearGroupHelper;
}

QVector<Item*> SudokuConstraint::affectedItems() const {
	QVector<Item*> items;
	foreach(ChoiceItem *item, m_items)
		items << item;
	return items;
}

void SudokuConstraint::setItems(const QVector<Item*> &items) {
	Q_ASSERT(!isInitialized());
	QVector<ChoiceItem*> newItems;
	newItems.reserve(items.size());
	foreach(Item *item, items) {
		ChoiceItem* choiceItem = dynamic_cast<ChoiceItem*>(item);
		if(!choiceItem) {
			setError("wrong item-type");
			return;
		}
		newItems << choiceItem;
	}
	m_items = newItems;
}

bool SudokuConstraint::addItem(Item *item) {
	Q_ASSERT(!m_initialized);
	ChoiceItem *choiceItem = dynamic_cast<ChoiceItem*>(item);
	if(!choiceItem) return false;
	m_items << choiceItem;
	return true;
}

void SudokuConstraint::init(Ruleset *rules) {
	Item::init(rules);
	m_initialized = true;
	m_lastInGroupHelper.setup(rules);
	m_clearGroupHelper.setup(rules);

	QVector<ChoiceItem*>::iterator it = m_items.begin();
	if(it == m_items.end()) {
		m_minValue = 0;
		m_maxValue = -1;
	} else {
		m_minValue = (*it)->minValue();
		m_maxValue = (*it)->maxValue();
		while(++it != m_items.end()) {
			int minValue = (*it)->minValue();
			int maxValue = (*it)->maxValue();
			if(minValue < m_minValue) {
				m_minValue = minValue;
			}
			if(maxValue > m_maxValue) {
				m_maxValue = maxValue;
			}
		}
	}
}

Constraint* SudokuConstraint::LastInGroupHelper::constraint() const {
	return m_constraint;
}

bool SudokuConstraint::LastInGroupHelper::apply(Problem* problem) const {
	QVector<ChoiceItem*> &items = m_constraint->m_items;
	for(int m = m_constraint->m_minValue; m <= m_constraint->m_maxValue; ++m) {
		int c = 0;
		ChoiceItem *item = 0;
		QVector<ChoiceItem*>::const_iterator it;
		for(it = items.begin(); it != items.end(); ++it) {
			if((*it)->minValue() > m || (*it)->maxValue() < m) continue;
			if((*it)->marker(problem, m)) {
				++c;
				item = *it;
			}
		}
		if(c == 1) {
			if(!item->value(problem)) {
				item->setValue(problem, m);
			} else {
				Q_ASSERT(item->value(problem) == m);
			}
		} else if(c == 0) {
			return false;
		}
	}
	return true;
}

Constraint* SudokuConstraint::ClearGroupHelper::constraint() const {
	return m_constraint;
}

bool SudokuConstraint::ClearGroupHelper::apply(Problem* problem) const {
	QVector<ChoiceItem*> &items = m_constraint->m_items;
	for(int m = m_constraint->m_minValue; m <= m_constraint->m_maxValue; ++m) {
		QVector<ChoiceItem*>::const_iterator it;
		for(it = items.begin(); it != items.end(); ++it) {
			if((*it)->value(problem) == m) {
				QVector<ChoiceItem*>::const_iterator it2;
				for(it2 = items.begin(); it2 != items.end(); ++it2) {
					if(it == it2) continue;
					if((*it2)->value(problem) == m) {
						// Value exists twice: against the rules
						return false;
					}
					if(m < (*it)->minValue() || m > (*it)->maxValue())
						continue;
					if((*it2)->marker(problem, m)) {
						// TODO this assert shouldn't be needed as
						// the problem should ensure this
						Q_ASSERT(!(*it2)->value(problem));
						(*it2)->setMarker(problem, m, false);
					}
				}
				break;
			}
		}
	}
	return true;
}
