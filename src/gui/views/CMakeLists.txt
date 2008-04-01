
set(ksudoku_views_SRCS
	ksview.cpp
	ArcBall.cpp
	gameactions.cpp
	renderer.cpp
	roxdokuview.cpp
	valuelistwidget.cpp
	view2d.cpp
)

kde4_add_library(ksudoku_views ${ksudoku_views_SRCS})

