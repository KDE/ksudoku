install( FILES
	4x4.desktop 4x4.xml
	Jigsaw.desktop Jigsaw.xml
	Samurai.desktop Samurai.xml
	TinySamurai.desktop TinySamurai.xml
	XSudoku.desktop XSudoku.xml
	DESTINATION  ${DATA_INSTALL_DIR}/ksudoku
)

kde4_install_icons(${DATA_INSTALL_DIR}/ksudoku/icons)
