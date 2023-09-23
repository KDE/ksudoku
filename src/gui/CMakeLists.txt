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
    KDEGames6
    KF6::Archive
    KF6::Crash
    KF6::GuiAddons
    KF6::KIOCore
    KF6::JobWidgets
    KF6::WidgetsAddons
    KF6::XmlGui
    Qt6::Core
    Qt6::Gui
    Qt6::PrintSupport
    Qt6::Svg
)

if(OPENGL_SUPPORT)
    target_link_libraries(ksudoku_gui
        Qt6::OpenGLWidgets
        ${OPENGL_LIBRARIES}
    )
endif(OPENGL_SUPPORT)
