include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QT_QTCORE_INCLUDE_DIR}
)

set(liblogine_SRCS
	choiceitem.cpp
	constraint.cpp
	constrainthelperstorage.cpp
	eliminationstorage.cpp
	item.cpp
	markerstorage.cpp
	problem.cpp
	ruleset.cpp
	solver.cpp
	statearray.cpp
	storage.cpp
	sudokuconstraint.cpp
)

qt4_automoc(${liblogine_SRCS})

kde4_add_library(logine STATIC ${liblogine_SRCS})

target_link_libraries(logine
	${QT_QTCORE_LIBRARY}
)

add_subdirectory(script)
