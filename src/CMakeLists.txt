find_package(OpenGL REQUIRED)

########### next target ###############

include_directories(
        ${CMAKE_SOURCE_DIR}/libkdegames
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/views
        ${CMAKE_CURRENT_SOURCE_DIR}/logic
        ${CMAKE_CURRENT_BINARY_DIR}/gui
        ${CMAKE_CURRENT_BINARY_DIR}/gui/views
#        ${CMAKE_CURRENT_BINARY_DIR}/gui/export
#        ${CMAKE_CURRENT_BINARY_DIR}/gui/export/draw
#        ${CMAKE_CURRENT_BINARY_DIR}/logic
	${OPENGL_INCLUDE_DIR}
)

set(ksudoku_SRCS
	main.cpp
)


kde4_add_executable(ksudoku ${ksudoku_SRCS})

target_link_libraries(ksudoku
	ksudoku_gui ksudoku_views ksudoku_logic
	kdegames
	 ${QT_QTOPENGL_LIBRARY} ${KDE4_KDEPRINT_LIBS} ${KDE4_KIO_LIBS} ${OPENGL_LIBRARIES} ${KDE4_KNEWSTUFF2_LIBRARY}
) #ksudoku_export ksudoku_draw

install(TARGETS ksudoku  DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############
ADD_SUBDIRECTORY( logic  )
ADD_SUBDIRECTORY( gui             )
ADD_SUBDIRECTORY( gui/views       )
#ADD_SUBDIRECTORY( gui/export      ) #TODO PORT
#ADD_SUBDIRECTORY( gui/export/draw ) #TODO PORT
ADD_SUBDIRECTORY( shapes )


#set(ksudoku_client_SRCS
#    ksudoku_client.cpp
#)

#kde3_automoc( ${ksudoku_client_SRCS} )

#kde3_add_executable(ksudoku_client ${ksudoku_client_SRCS})

#target_link_libraries(ksudoku_client ${QT_AND_KDECORE_LIBRARIES} kdeui)



#install(TARGETS ksudoku_client  DESTINATION ${BIN_INSTALL_DIR} )

########### install files ###############

install( FILES gui/ksudoku.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES gui/ksudokuui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksudoku )
install( FILES ksudokurc  DESTINATION ${CONFIG_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})


########### the old Makefile.am #########

# ## Makefile.am for ksudoku
#
# # this is the program that gets installed.  it's name is used for all
# # of the other Makefile.am variables
# bin_PROGRAMS = ksudoku ksudoku_client
#
# # set the include path for X, qt and KDE
# INCLUDES         = $(all_includes)
#
# # the library search path.
# ksudoku_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#
# # the libraries to link against.
# ksudoku_LDADD   = $(LIB_KFILE) $(LIB_KDEPRINT)  -lGL -lGLU
#
# # which sources should be compiled for ksudoku
# ksudoku_SOURCES = main.cpp ksudoku.cpp ksudokuview.cpp ksudokuiface.skel \
# 		qsudokubutton.cpp knewdlg.cpp sudoku_solver.cpp glwindow.cpp ArcBall.cpp
#
# # these are the headers for your project
# noinst_HEADERS = ksudoku.h ksudokuview.h qsudokubutton.h knewdlg.h \
# 		sudoku_solver.h glwindow.h
#
# # client stuff
# ksudoku_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
# ksudoku_client_LDADD   = $(LIB_KDECORE)
# ksudoku_client_SOURCES = ksudoku_client.cpp
#
# # let automoc handle all of the meta source files (moc)
# METASOURCES = AUTO
#
# messages: rc.cpp
# 	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
# 	$(XGETTEXT) *.cpp -o $(podir)/ksudoku.pot
#
# KDE_ICON = AUTO
#
# # this is where the kdelnk file will go
# kdelnkdir   = $(kde_appsdir)/Games
# kdelnk_DATA = ksudoku.desktop
#
# # this is where the XML-GUI resource file goes
# rcdir = $(kde_datadir)/ksudoku
# rc_DATA = ksudokuui.rc
