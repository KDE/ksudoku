include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(loscript_SRCS
	boardclass.cpp
	boardprototype.cpp
	boardwrapper.cpp
	itemclass.cpp
	itemprototype.cpp
	itemwrapper.cpp
	mocs.cpp
	objectmanager.cpp
	scriptedruleset.cpp
)

qt4_automoc(${loscript_SRCS})

add_library(loscript ${loscript_SRCS})

target_link_libraries(loscript
	logine
)
