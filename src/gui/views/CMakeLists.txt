
set(ksudoku_views_SRCS
	ksview.cpp
	ArcBall.cpp
	gameactions.cpp
	renderer.cpp
	valuelistwidget.cpp
	view2d.cpp
)

if(OPENGL_SUPPORT)
set(ksudoku_views_SRCS
	${ksudoku_views_SRCS}
	roxdokuview.cpp
)
endif(OPENGL_SUPPORT)

kde4_add_library(ksudoku_views ${ksudoku_views_SRCS})

# make sure settings.h is available (created in ksudoku_gui)
add_dependencies(ksudoku_views ksudoku_gui)

