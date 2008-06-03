#include "solverstate.h" 

#include "skgraph.h"

namespace ksudoku {

SolverState::SolverState(uint size, uint order)
	: m_size(size), m_order(order), m_values(size, 0), m_flags(order),
		m_remaining(size, order+1, order)
{
	for(int i = order-1; i >= 0; --i) {
		m_flags[i].detach();
		m_flags[i].fill(true, size);
	}
}

SolverState::SolverState(const SolverState& state)
	: m_size(state.m_size), m_order(state.m_order), m_values(state.m_values),
		m_flags(state.m_flags), m_remaining(state.m_remaining)
{
	for(int i = m_order-1; i >= 0; --i) {
		m_flags[i].detach();
	}
}

ProcessState SolverState::setValue(uint index, uint value, SKGraph* graph) {
	if(m_values[index] != 0) return KSS_CRITICAL;
	m_remaining.setValue(index, 0);
	m_values[index] = value;
	for(uint i = 0; i < (uint)graph->optimized_d[index]; ++i) {
		uint j = graph->optimized[index][i];
		if(m_values[j] == 0) {
			if(m_flags[value-1][j]) {
				m_flags[value-1].clearBit(j);
// 					if(m_remaining.value(j) == 8) printf("Aha\n");
				uint remaining = m_remaining.value(j);
				if(remaining == 1) return KSS_FAILURE;
				m_remaining.setValue(j, remaining-1);
			}
		}
	}
	return KSS_SUCCESS;
}

ProcessState SolverState::fill(const QVector<int>& values, SKGraph* graph) {
	for(int i = 0; i < m_size; ++i) {
		if(values[i] == 0) continue;
		if(setValue(i, values[i], graph) != KSS_SUCCESS)
			return KSS_FAILURE;
	}
	return KSS_SUCCESS;
}


/**
	* Sets all values for which only one flag is left
	* Returns whether it failed due to conflicts.
	*/
ProcessState SolverState::setAllDefindedValues(SKGraph* graph) {
	int index;
	ProcessState state;
	while((index = m_remaining.firstIndexWithValue(1)) >= 0) {
		for(int i = 0; ; ++i) {
			// Chekc whetere there wasn't a flag left
			if(i >= m_order) return KSS_CRITICAL;

			if(m_flags[i][index]) {
				if((state = setValue(index, i+1, graph)) != KSS_SUCCESS) return state;
				break;
			}
		}
	}
	return KSS_SUCCESS;
}

int SolverState::optimalSolvingIndex() {
	for(int i = 2; i <= m_order; ++i) {
		if(m_remaining.firstIndexWithValue(i) >= 0)
			return m_remaining.firstIndexWithValue(i);
	}
	return -1;
}

uint SolverState::possibleValue(uint index, uint startValue) {
	if(m_values[index] != 0) return 0;
	for(int i = startValue ? startValue-1 : 0; i < m_order; ++i) {
		if(m_flags[i][index])
			return i+1;
	}
	return 0;
}

}
