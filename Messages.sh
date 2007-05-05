#!/bin/sh
$EXTRACTRC $(find src/ -name "*.rc") >> rc.cpp || exit 11
$EXTRACTRC $(find src/ -name "*.ui") >> rc.cpp || exit 12
$XGETTEXT $(find src/ -name "*.cpp") rc.cpp -o $podir/ksudoku.pot

