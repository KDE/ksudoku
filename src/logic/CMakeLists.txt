set(ksudoku_logic_SRCS
	skgraph.cpp
	puzzle.cpp
)

add_library(ksudoku_logic STATIC ${ksudoku_logic_SRCS})
target_link_libraries(ksudoku_logic Qt${QT_MAJOR_VERSION}::Core)
