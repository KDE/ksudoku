
set(ksudoku_SRCS
	skgraph.cpp
	skbase.cpp
	skpuzzle.cpp
	sksolver.cpp
	puzzlefactory.cpp
	puzzle.cpp
	solver.cpp
)

add_library(ksudoku_logic ${ksudoku_SRCS})
