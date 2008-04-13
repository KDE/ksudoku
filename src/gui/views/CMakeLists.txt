
set(ksudoku_views_SRCS
	ksview.cpp
	ArcBall.cpp
	roxdokuview.cpp
	sudokuview.cpp
	qsudokubutton.cpp
	valuelistwidget.cpp
)

kde4_add_library(ksudoku_views ${ksudoku_views_SRCS})
# make sure settings.h is available (created in ksudoku_gui)
add_dependencies(ksudoku_views ksudoku_gui)

