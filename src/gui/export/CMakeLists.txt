
set(ksudoku_SRCS
	pagesize.cpp
	printdialogpage.cpp
	print.cpp
	exportdlg.cpp
	exportdlgsettings.cpp
	exportpreview.cpp
	exportpuzzles.cpp
	generateevent.cpp
)

set(moc_SRCS
	exportdlg.cpp
	exportpreview.cpp
	exportpuzzles.cpp
)

set(ksudoku_ui_files
	printdialogpagedlg.ui
	exportdlgbase.ui
)
KDE4_ADD_UI_FILES(ksudoku_SRCS ${ksudoku_ui_files})

INCLUDE_DIRECTORIES( draw ../../logic)

KDE4_ADD_LIBRARY(ksudoku_export ${ksudoku_SRCS})
