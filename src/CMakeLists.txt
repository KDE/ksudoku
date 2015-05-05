########### next target ###############

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/views
        ${CMAKE_CURRENT_SOURCE_DIR}/logic
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
)

kde4_add_app_icon(ksudoku_SRCS "${KDE4_ICON_INSTALL_DIR}/oxygen/*/apps/ksudoku.png")
kde4_add_executable(ksudoku ${ksudoku_SRCS})

target_link_libraries(ksudoku
	ksudoku_gui ksudoku_logic
	kdegames kdegamesprivate
	${KDE4_KIO_LIBS}
)

if(OPENGL_SUPPORT)
target_link_libraries(ksudoku
	${QT_QTOPENGL_LIBRARY} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS}
)
endif(OPENGL_SUPPORT)

install(TARGETS ksudoku  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### next target ###############
add_subdirectory( gui )
add_subdirectory( logic  )
add_subdirectory( shapes )
add_subdirectory( themes )


#set(ksudoku_client_SRCS
#    ksudoku_client.cpp
#)

#kde3_automoc( ${ksudoku_client_SRCS} )

#kde3_add_executable(ksudoku_client ${ksudoku_client_SRCS})

#target_link_libraries(ksudoku_client ${QT_AND_KDECORE_LIBRARIES} kdeui)



#install(TARGETS ksudoku_client  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS gui/ksudoku.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES gui/ksudokuui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksudoku )
install( FILES ksudokurc  DESTINATION ${CONFIG_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})


