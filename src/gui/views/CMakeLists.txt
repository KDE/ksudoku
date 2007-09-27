
set(ksudoku_views_SRCS
	ksview.cpp
	ArcBall.cpp
	roxdokuview.cpp
	sudokuview.cpp
	qsudokubutton.cpp
	valuelistwidget.cpp
)

kde4_add_library(ksudoku_views ${ksudoku_views_SRCS})

