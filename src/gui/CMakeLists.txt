

set(ksudoku_SRCS
	gameopt.cpp
	ksudoku.cpp
	ksudokugame.cpp
	serializer.cpp
	history.cpp
	symbols.cpp
	gameseldlg.cpp
# 	welcomewidget.cpp
)

INCLUDE_DIRECTORIES( views ksudoku_export)

qt4_automoc( ${ksudoku_SRCS} )

kde4_add_library(ksudoku_gui ${ksudoku_SRCS})
#constrains on build order:
ADD_DEPENDENCIES(ksudoku_gui ksudoku_views ksudoku_export)

########### next target ###############

set(ksudoku_client_SRCS
    ksudoku_client.cpp
)

#kde4_automoc( ${ksudoku_client_SRCS} )

#kde4_add_executable(ksudoku_client ${ksudoku_client_SRCS})

#target_link_libraries(ksudoku_client ${QT_AND_KDECORE_LIBRARIES} kdeui)

