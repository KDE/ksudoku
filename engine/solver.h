#ifndef _KSUDOKU_SOLVER_H_
#define _KSUDOKU_SOLVER_H_

#include <QVector>
#include <QBitArray>
#include <QSet>
#include <QStack>

#include "ruleset.h"
#include "problem.h"
#include "constraint.h"

class ForkHelper {
public:
	ForkHelper(const ForkHelper& h);
	static ForkHelper staticFork(Problem* problem);
	static ForkHelper randomFork(Problem* problem);
	IVariableItem *item() const;
	int firstValue() const;
	int nextValue(int value) const;
private:
	static IVariableItem *findForkNode(Problem* problem);
	ForkHelper(IVariableItem *item, int startValue, Problem* problem);
private:
	Problem* m_state;
	IVariableItem *m_item;
	int m_startValue;
};

class Solver {
public:
	enum State {
		Pending = 0,
		Success,
		Failure,
		NeedFork,
		PrepareFork,
		Finished
	};
	
public:
	Solver();
	
public:
	void loadEmpty(const Ruleset *rules);
	void loadProblem(const Problem *problem);
	
	void solve();

	void push();
	void pop();
	Problem &state();
	const Problem &state() const;
	
	QVector<Problem> results() const;

	int limit() const;
	void setLimit(int limit);
private:
	void solveFork(Problem& fork);
	void logicalSolve(Problem& state);
	ForkHelper findGoodFork(const Problem& state);
	
private:
	QVector<Problem> m_results;
	QStack<Problem> m_stack;
	State m_state;
	int m_limit;
};

#endif
