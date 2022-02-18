########### next target ###############

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/views
        ${CMAKE_CURRENT_SOURCE_DIR}/logic
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/gui
#        ${CMAKE_CURRENT_BINARY_DIR}/gui/export
#        ${CMAKE_CURRENT_BINARY_DIR}/gui/export/draw
        ${CMAKE_CURRENT_BINARY_DIR}/logic
        ${CMAKE_CURRENT_SOURCE_DIR}/generator
)

if(OPENGL_SUPPORT)
   include_directories(${OPENGL_INCLUDE_DIR})
endif(OPENGL_SUPPORT)

set(ksudoku_SRCS
	main.cpp
	generator/sudokuboard.cpp
	generator/state.cpp
	generator/dlxsolver.cpp
	generator/cagegenerator.cpp
	generator/mathdokugenerator.cpp
)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KSUDOKU VERSION_HEADER ksudoku_version.h)

ecm_qt_declare_logging_category(ksudoku_SRCS
    HEADER ksudoku_logging.h
    IDENTIFIER KSudokuLog
    CATEGORY_NAME org.kde.ksudoku
    DEFAULT_SEVERITY Warning
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*-apps-ksudoku.png")
ecm_add_app_icon(ksudoku_SRCS ICONS ${ICONS_SRCS})
add_executable(ksudoku ${ksudoku_SRCS})

target_link_libraries(ksudoku
    ksudoku_gui
    ksudoku_logic
    KF5KDEGames
    KF5KDEGamesPrivate
    KF5::KIOCore
    KF5::Crash
)

if(OPENGL_SUPPORT)
target_link_libraries(ksudoku
	Qt::OpenGL OpenGL::GL OpenGL::GLU ${CMAKE_DL_LIBS}
)
endif(OPENGL_SUPPORT)

install(TARGETS ksudoku  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### next target ###############
add_subdirectory(gui)
add_subdirectory(logic)
add_subdirectory(shapes)
add_subdirectory(themes)

########### install files ###############

install(PROGRAMS gui/org.kde.ksudoku.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES gui/org.kde.ksudoku.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES gui/ksudokuui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUIDIR}/ksudoku)
install(FILES ksudokurc  DESTINATION ${KDE_INSTALL_CONFDIR})

ecm_install_icons(ICONS
    128-apps-ksudoku.png
    16-apps-ksudoku.png
    32-apps-ksudoku.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
