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
	KF5::Crash
	KF5::GuiAddons
	KF5::KIOCore
	KF5::JobWidgets
	KF5::WidgetsAddons
	KF5::XmlGui
	Qt::Core 
	Qt::Gui
	Qt::PrintSupport
        Qt::Svg
	)

if(OPENGL_SUPPORT)
	target_link_libraries(ksudoku_gui 
		${OPENGL_LIBRARIES}
		Qt::OpenGL)
endif(OPENGL_SUPPORT)
