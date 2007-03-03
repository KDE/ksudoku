// part of KSUDOKU
#ifndef SKSolver_H
#define SKSolver_H

#include "skpuzzle.h"
#include "skgraph.h"

#include "ksudoku_types.h"

#define RANDOM(x) (int) (x*((float)rand())/(RAND_MAX+1.0f))
//j=1+(int) (10.0*rand()/(RAND_MAX+1.0));
//#define RANDOM(x) std::rand() % ((x))


// #define SUDOKU_HARDEST -1
// #define SUDOKU_HARD    0
// #define SUDOKU_MEDIUM  1
// #define SUDOKU_EASY    2
#define LEVINC         0
// 
#define SIMMETRY_RANDOM 0
#define SIMMETRY_NONE 1
#define SIMMETRY_DIAGONAL 2
#define SIMMETRY_CENTRAL 3
#define SIMMETRY_FOURWAY 4
// 
// #define SUDOKU  0
// #define ROXDOKU 1

#include <q3valuevector.h>

#include "solver.h"


using namespace ksudoku;

class Q3TextStream;


typedef unsigned int uint;

#include <math.h>
// SUDOKU PUZZLE SOLVER BASIC ALGORITHM - BY FRANCESCO ROSSI 2005 redsh@email.it
class SolverState;

/**
@author Francesco Rossi
*/

class SKSolver{
public:
	SKSolver(int n=9, bool threedimensionalf = false);
	~SKSolver();
	SKSolver(SKGraph* gr);
public:
	SKGraph* g       ;
	int      base    ;
	int      size    ;
	int      order   ;
	char     zerochar;
	
	//getters	
	GameType const type() const { return m_type; }
	
	//setters
	void setType(GameType const type) { m_type = type; }

private:
	GameType m_type  ;

public:
	int remove_numbers (SKPuzzle* p, int difficulty, int simmetry, int type);
	int remove_numbers2(SKPuzzle* p, int difficulty, int simmetry, int type);
	int solve (SKPuzzle* puzzle,  int max_solutions = 1, SKPuzzle* out_solutions = 0, int* forks=0);
	int solve2(SKPuzzle* puzzle,  int max_solutions = 1, SKPuzzle* out_solutions = 0, int* forks=0);

	int init();
// 	int fscc(bool* visited, bool* done, bool* mask, int node);
	void copy(SKPuzzle* dest, SKPuzzle* src);
	
	/**
	 * A simple and slow method for removing values from a completed puzzle.
	 * This method behaves essentialy like the implementation of version 0.3
	 */
	uint removeValuesSimple(Q3ValueVector<uint>& puzzle, uint hints, uint flags);
	
	/**
	 * A simple and fast method for removing values from a completed puzzle.
	 * It removes @p count values from puzzle according to @p flags.
	 */
	int removeValues(Q3ValueVector<uint>& puzzle, uint count, uint flags);

private:
	/**
	 * Completly resets all occurences of @p value (and their symmetric complements) to 0.
	 * This method only changes @p puzzle when it remains solveable.
	 * @returns count of removed values or 0 if the puzzle con't be solved afterwards.
	 */
	uint removeValueCompletely(Q3ValueVector<uint>& puzzle, uint value, uint flags);
	
	/**
	 * Resets value at @p index (and its symmetric complements) to 0.
	 * This method only changes @p puzzle when it remains solveable.
	 * @returns count of removed values or 0 if the puzzle can't be solved afterwards.
	 */
	uint removeAtIndex(Q3ValueVector<uint>& puzzle, uint index, uint flags);
	
	ProcessState solveEngine(::SolverState& state, SKPuzzle* puzzle, uint* solutionsLeft, uint* forksLeft);
	
	/**
	 */
	uint getSymmetry(uint flags, uint index, uint out[4]);
	
	int solve_engine(SKPuzzle *s,  int& solutions, SKPuzzle* out_solutions = 0, int maxsolutions = 1, int last_add=-1, int dynindex=-1, int dynvalue=0, int* forks=0);
	void addConnection(int i, int j);
	int get_simmetric(int order, int size, int type, int idx, int which, int out[4]);
};


#endif
