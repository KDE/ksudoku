include_directories( views )

set(ksudoku_SRCS
	config.cpp
	ksudoku.cpp
	ksudokugame.cpp
	serializer.cpp
	history.cpp
	symbols.cpp
	gamevariants.cpp
	welcomescreen.cpp
)

set(ksudoku_views_SRCS
	views/ksview.cpp
	views/gameactions.cpp
	views/renderer.cpp
	views/valuelistwidget.cpp
	views/view2d.cpp )

if(OPENGL_SUPPORT)
	set(ksudoku_views_SRCS
		${ksudoku_views_SRCS}
		views/ArcBall.cpp
		views/roxdokuview.cpp )
endif(OPENGL_SUPPORT)


kde4_add_ui_files(ksudoku_SRCS welcomescreen.ui configgame.ui)

kde4_add_kcfg_files(ksudoku_SRCS settings.kcfgc )

kde4_add_library(ksudoku_gui STATIC ${ksudoku_SRCS} ${ksudoku_views_SRCS})

target_link_libraries(ksudoku_gui 
	ksudoku_logic
    kdegames
	${KDE4_KDEUI_LIBS} 
	${KDE4_KDECORE_LIBS} 
	${KDE4_KIO_LIBRARY}
	${QT_QTCORE_LIBRARY} 
	${QT_QTGUI_LIBRARY}
	)

if(OPENGL_SUPPORT)
	target_link_libraries(ksudoku_gui 
		${OPENGL_LIBRARIES}
		${QT_QTOPENGL_LIBRARY})
endif(OPENGL_SUPPORT)

	
########### next target ###############

set(ksudoku_client_SRCS
    ksudoku_client.cpp
)

#kde4_add_executable(ksudoku_client ${ksudoku_client_SRCS})

#target_link_libraries(ksudoku_client ${QT_AND_KDECORE_LIBRARIES} kdeui)

