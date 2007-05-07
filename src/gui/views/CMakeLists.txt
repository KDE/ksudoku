
set(ksudoku_views_SRCS
	ksview.cpp
	ArcBall.cpp
	roxdokuview.cpp
	ksudokuview.cpp
	qsudokubutton.cpp
)

kde4_automoc( ${ksudoku_views_SRCS} )
kde4_add_library(ksudoku_views ${ksudoku_views_SRCS})

