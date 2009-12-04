include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(liblogine_SRCS
	choiceitem.cpp
	constraint.cpp
	constrainthelperstorage.cpp
	eliminationstorage.cpp
	item.cpp
	markerstorage.cpp
	mocs.cpp
	problem.cpp
	ruleset.cpp
	solver.cpp
	statearray.cpp
	storage.cpp
	sudokuconstraint.cpp
)

qt4_automoc(${liblogine_SRCS})

add_library(logine ${liblogine_SRCS})

target_link_libraries(logine
	${QT_LIBRARIES}
	${QT_QTCORE_LIBRARY}
	${QT_QTSCRIPT_LIBRARY}
)

add_subdirectory(script)
