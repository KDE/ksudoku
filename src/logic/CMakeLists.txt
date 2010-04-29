set(ksudoku_logic_SRCS
	skgraph.cpp
	puzzle.cpp
)

kde4_add_library(ksudoku_logic STATIC ${ksudoku_logic_SRCS})
target_link_libraries(ksudoku_logic ${QT_QTCORE_LIBRARY})
