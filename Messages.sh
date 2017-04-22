#!/bin/sh
$EXTRACTRC $(find src/ -name "*.rc" -o -name "*.ui" -o -name "*.kcfg") >> rc.cpp
$XGETTEXT $(find src/ -name "*.cpp") rc.cpp -o $podir/ksudoku.pot
rm -f rc.cpp
