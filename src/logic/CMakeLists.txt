set(ksudoku_logic_SRCS
	skgraph.cpp
	skbase.cpp
	skpuzzle.cpp
	sksolver.cpp
	puzzlefactory.cpp
	puzzle.cpp
	solver.cpp
	solverstate.cpp
)

kde4_add_library(ksudoku_logic ${ksudoku_logic_SRCS})
target_link_libraries(ksudoku_logic ${QT_QTCORE_LIBRARY})
