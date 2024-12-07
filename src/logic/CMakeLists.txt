set(ksudoku_logic_SRCS
	skgraph.cpp skgraph.h
	puzzle.cpp puzzle.h
)

add_library(ksudoku_logic STATIC ${ksudoku_logic_SRCS})
target_link_libraries(ksudoku_logic Qt6::Core)
