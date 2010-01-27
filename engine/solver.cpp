#include "solver.h"

#include <iostream>

#include "eliminationstorage.h"

// NOTE this is only a hack to make it possible to reset markers
#include "markerstorage.h"
// NOTE this is only a hack to make it possible to reset elimination data
#include "eliminationstorage.h"
// NOTE this is only a hack to make it possible to reset constraint helpers
#include "constrainthelperstorage.h"

ForkHelper::ForkHelper(const ForkHelper& h)
	: m_state(h.m_state), m_item(h.m_item), m_startValue(h.m_startValue)
{ }

ForkHelper ForkHelper::staticFork(Problem* problem) {
	IVariableItem *item = findForkNode(problem);
	if(!item) return ForkHelper(0, -1, problem);

	int startValue = item->nextPossibility(problem);
	
	return ForkHelper(item, startValue, problem);
}

ForkHelper ForkHelper::randomFork(Problem* problem) {
	IVariableItem *item = findForkNode(problem);
	if(!item) return ForkHelper(0, -1, problem);

	int possibilities = item->possibilities(problem);
	int skips = rand() % possibilities;

	int startValue = item->nextPossibility(problem);
	for(int i = 0; i < skips; ++i) {
		startValue = item->nextPossibility(problem, startValue);
	}

	return ForkHelper(item, startValue, problem);
}

IVariableItem *ForkHelper::item() const {
	return m_item;
}

int ForkHelper::firstValue() const {
	return m_startValue;
}

int ForkHelper::nextValue(int value) const {
	value = m_item->nextPossibility(m_state, value);
	if(value == -1) value = m_item->nextPossibility(m_state);
	if(value == m_startValue) return -1;
	return value;
}

IVariableItem *ForkHelper::findForkNode(Problem* problem) {
	const EliminationStorage *storage = ::storage<const EliminationStorage>(problem->ruleset());
	if(!storage) return 0;
	return storage->itemWithLeastPossibilitiesLeft(problem);
}

ForkHelper::ForkHelper(IVariableItem *item, int startValue, Problem* problem)
	: m_state(problem), m_item(item), m_startValue(startValue)
{ }

Solver::Solver() {
	// TODO set limit through property
	m_limit = 1;
	m_stack.push(Problem());
}

void Solver::loadEmpty(const Ruleset *rules) {
	// TODO specify that this problem is a solver problem
	m_stack.top() = Problem(rules);
}

void Solver::loadProblem(const Problem *problem) {
	// TODO specify that this problem is a solver problem
	m_stack.top() = *problem;
	// NOTE resets to solver-specific storages
	storage<const MarkerStorage>(m_stack.top().ruleset())->reset(&m_stack.top());
	storage<const EliminationStorage>(m_stack.top().ruleset())->reset(&m_stack.top());
	storage<const ConstraintHelperStorage>(m_stack.top().ruleset())->reset(&m_stack.top());
}

QVector<Problem> Solver::results() const {
	return m_results;
}

void Solver::solve() {
	Problem problem(m_stack.top());

// 	Graph* graph = m_argument.graph();
// 	// TODO no longer needed as its the job of the loaded graph to ensure its solveability
// 	for(int i = graph->size()-1; i >= 0; --i) {
// 		int val = state.value(i);
// 		for(int j = graph->valueCount()-1; j >= 0; --j) {
// 			state.setMarker(i, j, val==0 || val==j+1);
// 		}
// 	}

	m_results.clear();
	
	m_state = Pending;

	solveFork(problem);
	return;
}

void Solver::push() {
	m_stack.push(m_stack.top());
}

void Solver::pop() {
	m_stack.pop();
}

Problem &Solver::state() {
	return m_stack.top();
}

const Problem &Solver::state() const {
	return m_stack.top();
}

int Solver::limit() const {
	return m_limit;
}

void Solver::setLimit(int limit) {
	m_limit = limit;
}

void Solver::solveFork(Problem &fork) {
	logicalSolve(fork);

	if(m_state != Success && m_state != Failure) {
		ForkHelper helper(ForkHelper::randomFork(&fork));
		IVariableItem *item = helper.item();
		if(!item) {
			m_state = Failure;
			return;
		}
		for(int value = helper.firstValue(); value > 0; value = helper.nextValue(value)) {
			Problem subFork(fork);
			item->applyPossibility(&subFork, value);

			solveFork(subFork);
			m_state = Pending;
			
			if(m_results.size() >= m_limit)
				break;
		}
	}
}

void Solver::logicalSolve(Problem& problem) {
	const EliminationStorage *storage = ::storage<const EliminationStorage>(problem.ruleset());
	const ConstraintHelperStorage *helperStorage = ::storage<const ConstraintHelperStorage>(problem.ruleset());
	while(const ConstraintHelper* helper = helperStorage->firstActiveHelper(&problem)) {
		if(!helper->resolve(&problem)) {
			m_state = Failure;
			return;
		}
		
		if(m_state == Failure) {
			return;
		}
		if(helperStorage->activeHelpers(&problem) == 0 && storage->isFinished(&problem)) {
			// TODO specify that this problem is a minimalistic problem
			m_results << problem;
			m_state = Success;
			return;
		}
	}
}
