
set(ksudoku_SRCS
	drawbase.cpp
	sudokudraw.cpp
	roxdokudraw.cpp
	drawfactory.cpp
)


ADD_LIBRARY(ksudoku_draw ${ksudoku_SRCS})
