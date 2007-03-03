project(ksudoku)

##options
#OPTION(DEBUG
#	"Build the project using debugging code"
#	ON  #default on, make default off in stable branch
#	CMAKE_BUILD_TYPE "DEBUG"
#	#if CMAKE_BUILD_TYPE is not defined, FindKDE3.make
#	#adds -O2 to CMAKE_CXX_FLAGS
#)

#find_package(Qt3  REQUIRED) # find and setup Qt3 for this project
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories ( ${QDBUS_INCLUDE_DIRS} ${QT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
# tell cmake where to search for libraries:
#link_directories(${KDE4_LIB_DIR})

IF(DEBUG)
	ADD_DEFINITIONS(-DDEBUG -ggdb -O)
ENDIF(DEBUG)

# tell cmake where to search for Qt/KDE headers:
include_directories(
	${KDE3_INCLUDE_DIR}
	${QT_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/src/gui
	${CMAKE_SOURCE_DIR}/src/gui/views
	${CMAKE_SOURCE_DIR}/src/logic
	${CMAKE_BINARY_DIR}/src
	${CMAKE_BINARY_DIR}/src/gui
	${CMAKE_BINARY_DIR}/src/gui/views
	${CMAKE_BINARY_DIR}/src/gui/export
	${CMAKE_BINARY_DIR}/src/gui/export/draw
	${CMAKE_BINARY_DIR}/src/logic
)

add_subdirectory( doc )
add_subdirectory( po )
add_subdirectory( src )

message(STATUS "DEBUG: ${DEBUG}")
message(STATUS "intstall prefix: ${CMAKE_INSTALL_PREFIX}")
