#!/bin/bash
EXE=""
if test "$1"  == "-g"
then
    echo running with GDB
    shift
    EXE="gdb --args"
fi
$EXE go4analysis -disable-asf -lib libGo4UserAnalysis.so $@
