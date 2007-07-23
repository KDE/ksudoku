project(ksudoku)

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

add_subdirectory( src )

message(STATUS "ksudoku: DEBUG: ${DEBUG}")
message(STATUS "ksudoku: install prefix: ${CMAKE_INSTALL_PREFIX}")
