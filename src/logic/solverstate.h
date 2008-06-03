#ifndef _SOLVERSTATE_H_
#define _SOLVERSTATE_H_

#include <QVector>
#include <QBitArray>
#include "grouplookup.h" // TODO Remove (only needed by SolverState)

class SKGraph;

namespace ksudoku {

enum ProcessState {
	/// An operation finished successfuly
	KSS_SUCCESS          = 0,
	/// An operation failed, this may (dependant on op) mean that there is no solution
	KSS_FAILURE          = 1,
	/// An operation stopped, as no more solutions are required
	KSS_ENOUGH_SOLUTIONS = 2,
	/// To count of maximum forks was exceeded
	KSS_ENOUGH_FORKS     = 3,
	/// There is an internal failure
	KSS_CRITICAL         = 4
};

class SolverState {
public:
	SolverState(uint size, uint order);

	SolverState(const SolverState& state);;

	uint value(uint index) const { return m_values[index]; }

	ProcessState setValue(uint index, uint value, SKGraph* graph);
	ProcessState fill(const QVector<int>& values, SKGraph* graph);

	/*
	 * Sets all values for which only one flag is left
	 * Returns whether it failed due to conflicts.
	 */
	ProcessState setAllDefindedValues(SKGraph* graph);
	int optimalSolvingIndex();
	uint possibleValue(uint index, uint startValue = 0);

private:
	int                m_size;
	int                m_order;
	QVector<uint>      m_values;
	QVector<QBitArray> m_flags; // I don't know whether this is fast enough
	GroupLookup        m_remaining;
};

}

#endif
