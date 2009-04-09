#!/bin/sh
$EXTRACTRC $(find src/ -name "*.rc") >> rc.cpp || exit 11
$EXTRACTRC $(find src/ -name "*.ui") >> rc.cpp || exit 12
$EXTRACTRC $(find src/ -name "*.kcfg") >> rc.cpp
$XGETTEXT $(find src/ -name "*.cpp") rc.cpp -o $podir/ksudoku.pot
rm -f rc.cpp
