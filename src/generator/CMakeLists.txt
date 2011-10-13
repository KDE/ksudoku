
########### next target ###############

set(mysudoku_SRCS
    main.cpp 
    sudoku.cpp 
    sudokuboard.cpp
    plainsudokuboard.cpp
    samuraiboard.cpp
    roxdokuboard.cpp
    state.cpp
    )

# kde4_add_app_icon(kgoldrunner_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-kgoldrunner.png")
kde4_add_executable(mysudoku ${mysudoku_SRCS})

target_link_libraries(mysudoku ${KDE4_KDEUI_LIBS} kdegames)

install(TARGETS mysudoku ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

# install( PROGRAMS KGoldrunner.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
# install( FILES kgoldrunnerui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kgoldrunner )

# kde4_install_icons( ${ICON_INSTALL_DIR} )





