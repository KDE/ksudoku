#include "constraint.h" 

#include "ruleset.h"

#include <iostream>

/**
 * \class ConstraintHelper
 *
 * \brief ConstraintHelper is the base for all constraint helpers.
 *
 * Constraint-helpers do the actual validation against the rules of a constraint
 * and optionaly helps to resolve the constraint
 */

/**
 * \fn void apply(IHelperTarget* target) const
 * Applies the helper to the \a target (which is a solver in most cases).
 *
 * This includes validation and changing the targets problem in a way the limits
 * the possible configurations towards the solution(s) of the problem.
 * Needs to be implemented by derived classes.
 */

/**
 * \fn Constraint* constraint() const
 * Returns the constraint the helper belongs to.
 *
 * Needs to be implemented by derived classes.
 */

/**
 * Constructs a new constraint-helper.
 */
ConstraintHelper::ConstraintHelper()
	: m_entry(this)
{
}

/**
 * Registers the helper at \a ruleset.
 */
void ConstraintHelper::setup(Ruleset* ruleset) {
	ruleset->addHelper(this);
	m_entry.setup(ruleset);
}

/**
 * Called when an \a item associated with the helper changed.
 *
 * Reactivates the helper in \a problem if it is really affected
 * by the change in item.
 */
void ConstraintHelper::itemChanged(Problem* problem, const Item* item) const {
	if(!constraint()->reallyAffectsItem(item, problem)) return;
	
	m_entry.activate(problem);
}

/**
 * Helps to resolve the constraint.
 *
 * This method calls apply() and afterwards set this helper to idle
 */
bool ConstraintHelper::resolve(Problem *problem) const {
	bool noerror = apply(problem);
	m_entry.resolve(problem);
	return noerror;
}


/**
 * \class Constraint
 *
 * \brief Constraint is the base for all constraints in a graph.
 *
 * A constraint is the specific application of a rule. For example
 * in sudoku a constraint may limit all cells in a specific row
 * to contain each value only once.
 */

/**
 * \fn QVector<int> Constraint::affectedNodes() const
 * Returns a list of all nodes that may be affected by the constraint.
 */

/**
 * Constructs a new constraint.
 */
Constraint::Constraint() {
}

/**
 * Returns true if the constraint really affects \a item.
 * Reimplement this if the nodes affected by constraint might change 
 * depending on the state of \a puzzle.
 */
bool Constraint::reallyAffectsItem(const Item *item, Problem *puzzle) const {
	Q_UNUSED(item);
	Q_UNUSED(puzzle);
	return true;
}

/**
 * Reimplement this to initilazies the constraint and registrater it
 * to the graph.
 */
void Constraint::init(Ruleset *rules) {
	Q_UNUSED(rules);
}
