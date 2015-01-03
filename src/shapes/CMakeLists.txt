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
	DESTINATION  ${DATA_INSTALL_DIR}/ksudoku
)

ecm_install_icons(ICONS
hi128-action-ksudoku-jigsaw.png         hi16-action-ksudoku-jigsaw.png         hi22-action-ksudoku-jigsaw.png         hi32-action-ksudoku-jigsaw.png         hi48-action-ksudoku-jigsaw.png         hi64-action-ksudoku-jigsaw.png
hi128-action-ksudoku-ksudoku_16x16.png  hi16-action-ksudoku-ksudoku_16x16.png  hi22-action-ksudoku-ksudoku_16x16.png  hi32-action-ksudoku-ksudoku_16x16.png  hi48-action-ksudoku-ksudoku_16x16.png  hi64-action-ksudoku-ksudoku_16x16.png
hi128-action-ksudoku-ksudoku_25x25.png  hi16-action-ksudoku-ksudoku_25x25.png  hi22-action-ksudoku-ksudoku_25x25.png  hi32-action-ksudoku-ksudoku_25x25.png  hi48-action-ksudoku-ksudoku_25x25.png  hi64-action-ksudoku-ksudoku_25x25.png
hi128-action-ksudoku-ksudoku_4x4.png    hi16-action-ksudoku-ksudoku_4x4.png    hi22-action-ksudoku-ksudoku_4x4.png    hi32-action-ksudoku-ksudoku_4x4.png    hi48-action-ksudoku-ksudoku_4x4.png    hi64-action-ksudoku-ksudoku_4x4.png
hi128-action-ksudoku-ksudoku_9x9.png    hi16-action-ksudoku-ksudoku_9x9.png    hi22-action-ksudoku-ksudoku_9x9.png    hi32-action-ksudoku-ksudoku_9x9.png    hi48-action-ksudoku-ksudoku_9x9.png    hi64-action-ksudoku-ksudoku_9x9.png
hi128-action-ksudoku.png                hi16-action-ksudoku.png                hi22-action-ksudoku.png                hi32-action-ksudoku.png                hi48-action-ksudoku.png                hi64-action-ksudoku.png
hi128-action-ksudoku-roxdoku_3x3x3.png  hi16-action-ksudoku-roxdoku_3x3x3.png  hi22-action-ksudoku-roxdoku_3x3x3.png  hi32-action-ksudoku-roxdoku_3x3x3.png  hi48-action-ksudoku-roxdoku_3x3x3.png  hi64-action-ksudoku-roxdoku_3x3x3.png
hi128-action-ksudoku-roxdoku_4x4x4.png  hi16-action-ksudoku-roxdoku_4x4x4.png  hi22-action-ksudoku-roxdoku_4x4x4.png  hi32-action-ksudoku-roxdoku_4x4x4.png  hi48-action-ksudoku-roxdoku_4x4x4.png  hi64-action-ksudoku-roxdoku_4x4x4.png
hi128-action-ksudoku-roxdoku_5x5x5.png  hi16-action-ksudoku-roxdoku_5x5x5.png  hi22-action-ksudoku-roxdoku_5x5x5.png  hi32-action-ksudoku-roxdoku_5x5x5.png  hi48-action-ksudoku-roxdoku_5x5x5.png  hi64-action-ksudoku-roxdoku_5x5x5.png
hi128-action-ksudoku-samurai.png        hi16-action-ksudoku-samurai.png        hi22-action-ksudoku-samurai.png        hi32-action-ksudoku-samurai.png        hi48-action-ksudoku-samurai.png        hi64-action-ksudoku-samurai.png
hi128-action-ksudoku-tiny_samurai.png   hi16-action-ksudoku-tiny_samurai.png   hi22-action-ksudoku-tiny_samurai.png   hi32-action-ksudoku-tiny_samurai.png   hi48-action-ksudoku-tiny_samurai.png   hi64-action-ksudoku-tiny_samurai.png
hi128-action-ksudoku-xsudoku.png        hi16-action-ksudoku-xsudoku.png        hi22-action-ksudoku-xsudoku.png        hi32-action-ksudoku-xsudoku.png        hi48-action-ksudoku-xsudoku.png        hi64-action-ksudoku-xsudoku.png
DESTINATION ${DATA_INSTALL_DIR}/ksudoku/icons)
