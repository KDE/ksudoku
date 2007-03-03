#include "puzzle.h"
#include <cstdlib>
#include <time.h>
#include <qstring.h>

namespace ksudoku {

Puzzle::Puzzle(SKSolver* solver, bool withSolution)
	: m_withSolution(withSolution)
	, m_puzzle(0)
	, m_solution(0)
	, m_solver(solver)
	, m_difficulty(0)
	, m_symmetry(0)
{ }

Puzzle::~Puzzle() {
	delete m_puzzle;
	delete m_solution;

	m_puzzle   = 0;
	m_solution = 0;
}

bool Puzzle::init() {
	if(m_puzzle)
		return false;
	
	if(m_withSolution)
		return false;

	m_puzzle = new SKPuzzle(m_solver->g->order, m_solver->g->type);

	for(uint i = 0; i < (uint)m_puzzle->size; ++i)
		m_puzzle->numbers[i] = 0;
// 		m_puzzle->setValue(i, 0);

	return true;
}

bool Puzzle::init(int difficulty, int symmetry) {
	if(m_puzzle)
		return false;
	
	SKPuzzle* puzzle = new SKPuzzle(m_solver->g->order, m_solver->g->type, m_solver->g->size);
	
	if(!puzzle)
		return false;
	
	printf("init %d %d\n", difficulty, symmetry);

//	std::srand( time(0) );
	m_solver->solve(puzzle, 1, puzzle);

	SKPuzzle* solution = 0;
	if(m_withSolution) {
		solution = new SKPuzzle(m_solver->g->order, m_solver->g->type);
		
		if(!solution) {
			delete puzzle;
			return false;
		}
		
		m_solver->copy(solution, puzzle);
	}
	
	m_solver->remove_numbers(puzzle, difficulty, symmetry, m_solver->g->type); //why was it 1?
	m_difficulty = difficulty;
	m_symmetry   = symmetry  ;
	
	m_puzzle   = puzzle  ;
	m_solution = solution;
	return true;
}

int Puzzle::init(const QByteArray& values, int* forks) {
	if(m_puzzle)
		return -1;
	
	SKPuzzle* puzzle   = new SKPuzzle(m_solver->g->order, m_solver->type());
	SKPuzzle* solution = new SKPuzzle(m_solver->g->order, m_solver->type());
	
	if(!(puzzle && solution))
		return -1;
	
	for(uint i = 0; i < (uint)m_solver->g->size; ++i)
		puzzle->numbers[i] = values[i];
// 		puzzle->setValue(i, values[i]);
	
// 	m_solver->copy(solution, puzzle);
	
	int success = m_solver->solve(puzzle, 1, solution, forks);
	if(success == 0) {
		delete puzzle;
		delete solution;
		return 0;
	}
	
	success = m_solver->solve(puzzle, 2);
	
	m_puzzle = puzzle;
	if(m_withSolution)
		m_solution = solution;
	else
		delete solution;
	
	return success;
}

bool Puzzle::init(const QByteArray& values, const QByteArray& solutionValues) {
	if(m_puzzle)
		return false;
	
	
	SKPuzzle* puzzle = new SKPuzzle(m_solver->g->order, m_solver->type());
	if(!puzzle)
		return false;
	
	for(uint i = 0; i < (uint)m_solver->g->size; ++i)
		puzzle->numbers[i] = values[i];
// 		puzzle->setValue(i, values[i]);
	
	if(solutionValues.count() != 0) {
		SKPuzzle* solution = new SKPuzzle(m_solver->g->order, m_solver->type());
		if(!solution) {
			delete puzzle;
			return false;
		}
		
		for(uint i = 0; i < (uint)m_solver->g->size; ++i)
			solution->numbers[i] = solutionValues[i];
// 			solution->setValue(i, solutionValues[i]);
		
		m_solution = solution;
	}
	m_puzzle = puzzle;
	return true;
}


// QChar Puzzle::value2Char(uint value) const {
// 	// NOTE calls to this method are allowed before init
// 	return value + m_solver->g->zerochar;
// }
// 
// int Puzzle::char2Value(QChar c) const{
// 	// NOTE calls to this method are allowed before init
// 	int val = c.lower();
// 	if(val < m_solver->g->zerochar) return -1;
// 	val -= m_solver->g->zerochar;
// 	if((uint)val > (uint)m_solver->order) return -1;
// 	return val;
// }

}
