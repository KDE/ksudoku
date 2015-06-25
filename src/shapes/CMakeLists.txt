install( FILES
	4x4.desktop 4x4.xml
	Tetromino.desktop Tetromino.xml
	Pentomino.desktop Pentomino.xml
	6x6.desktop 6x6.xml
	Jigsaw.desktop Jigsaw.xml
	Samurai.desktop Samurai.xml
	TinySamurai.desktop TinySamurai.xml
	XSudoku.desktop XSudoku.xml
	Aztec.desktop Aztec.xml
	Nonomino.desktop Nonomino.xml
	Sohei.desktop Sohei.xml
	Windmill.desktop Windmill.xml
	DoubleRoxdoku.desktop DoubleRoxdoku.xml
	RoxdokuTwin.desktop RoxdokuTwin.xml
	SamuraiRoxdoku.desktop SamuraiRoxdoku.xml
	Mathdoku_4x4.desktop Mathdoku_4x4.xml
	Mathdoku_Settable.desktop Mathdoku_Settable.xml
	Killer_4x4.desktop Killer_4x4.xml
	# Killer_9x9.desktop Killer_9x9.xml
	DESTINATION  ${DATA_INSTALL_DIR}/ksudoku
)

kde4_install_icons(${DATA_INSTALL_DIR}/ksudoku/icons)
