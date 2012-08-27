project(ksudoku)

if(NOT COMPILING_KDEGAMES)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	include(MacroLibrary)
	find_package(KDEGames REQUIRED)

	macro_optional_find_package(OpenGL)
	macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3Dgraphics applications" "http://mesa3d.sourceforge.net" FALSE "" "Kubrick will not be built and KSudoku will not have Roxdoku support without OpenGL.")
	macro_log_feature(QT_QTOPENGL_FOUND "QtOpenGL" "Qt module that make it easy to use OpenGL in Qt apps" "http://doc.trolltech.com/latest/qtopengl.html" FALSE "" "Kubrick will not be built and KSudoku will not have Roxdoku support without QtOpenGL.")
	macro_display_feature_log()

	if(OPENGL_FOUND AND QT_QTOPENGL_FOUND)
		set(OPENGL_SUPPORT true)
	endif(OPENGL_FOUND AND QT_QTOPENGL_FOUND)

	add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
	include_directories(${KDEGAMES_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QDBUS_INCLUDE_DIRS})
endif(NOT COMPILING_KDEGAMES)

##options
#OPTION(DEBUG
#	"Build the project using debugging code"
#	ON  #default on, make default off in stable branch
#	CMAKE_BUILD_TYPE "DEBUG"
#	#if CMAKE_BUILD_TYPE is not defined, FindKDE3.make
#	#adds -O2 to CMAKE_CXX_FLAGS
#)

IF(DEBUG)
	ADD_DEFINITIONS(-DDEBUG -ggdb -O)
ENDIF(DEBUG)

# enable code depending on OPENGL-support
if(OPENGL_SUPPORT)
	add_definitions(-DOPENGL_SUPPORT)
endif(OPENGL_SUPPORT)

add_subdirectory( src )
add_subdirectory( doc )

#message(STATUS "ksudoku: DEBUG: ${DEBUG}")
#message(STATUS "ksudoku: install prefix: ${CMAKE_INSTALL_PREFIX}")
