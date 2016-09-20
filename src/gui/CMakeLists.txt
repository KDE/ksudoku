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
	puzzleprinter.cpp
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


ki18n_wrap_ui(ksudoku_SRCS welcomescreen.ui configgame.ui)

kconfig_add_kcfg_files(ksudoku_SRCS settings.kcfgc )

add_library(ksudoku_gui STATIC ${ksudoku_SRCS} ${ksudoku_views_SRCS})

target_link_libraries(ksudoku_gui 
	ksudoku_logic
    KF5KDEGames
        KF5::Archive	 
	KF5::KDELibs4Support 
	KF5::KIOCore
	Qt5::Core 
	Qt5::Gui
        Qt5::Svg
	)

if(OPENGL_SUPPORT)
	target_link_libraries(ksudoku_gui 
		${OPENGL_LIBRARIES}
		Qt5::OpenGL)
endif(OPENGL_SUPPORT)

	
########### next target ###############

set(ksudoku_client_SRCS
    ksudoku_client.cpp
)

#add_executable(ksudoku_client ${ksudoku_client_SRCS})

#target_link_libraries(ksudoku_client ${QT_AND_KDECORE_LIBRARIES} kdeui)

