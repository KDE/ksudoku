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
	KF${KF_MAJOR_VERSION}::Archive
	KF${KF_MAJOR_VERSION}::Crash
	KF${KF_MAJOR_VERSION}::GuiAddons
	KF${KF_MAJOR_VERSION}::KIOCore
	KF${KF_MAJOR_VERSION}::JobWidgets
	KF${KF_MAJOR_VERSION}::WidgetsAddons
	KF${KF_MAJOR_VERSION}::XmlGui
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::PrintSupport
        Qt${QT_MAJOR_VERSION}::Svg
	)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(ksudoku_gui KDEGames6 Qt6::OpenGLWidgets)
else()
    target_link_libraries(ksudoku_gui KF5KDEGames)
endif()

if(OPENGL_SUPPORT)
    if (QT_MAJOR_VERSION STREQUAL "6")
	target_link_libraries(ksudoku_gui
	    Qt6::OpenGLWidgets
	)
    else()
	target_link_libraries(ksudoku_gui 
	    ${OPENGL_LIBRARIES}
        )
    endif()
endif(OPENGL_SUPPORT)
