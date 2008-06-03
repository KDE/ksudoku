
set(ksudoku_SRCS
	skgraph.cpp
	skbase.cpp
	skpuzzle.cpp
	sksolver.cpp
	puzzlefactory.cpp
	puzzle.cpp
	solver.cpp
	solverstate.cpp
)

kde4_add_library(ksudoku_logic ${ksudoku_SRCS})
# make sure settings.h is available (created in ksudoku_gui)
add_dependencies(ksudoku_logic ksudoku_gui)
