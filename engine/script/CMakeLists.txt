include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QT_QTCORE_INCLUDE_DIR}
	${QT_QTSCRIPT_INCLUDE_DIR}
)

set(loscript_SRCS
	boardclass.cpp
	boardprototype.cpp
	boardwrapper.cpp
	itemclass.cpp
	itemprototype.cpp
	itemwrapper.cpp
	objectmanager.cpp
	scriptedruleset.cpp
)

kde4_add_library(loscript STATIC ${loscript_SRCS})

target_link_libraries(loscript
	logine
	${QT_QTSCRIPT_LIBRARY}
)
