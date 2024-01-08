#!/bin/bash
source /u/land/fake_cvmfs/sourceme.sh ""
source go4login 

EXE=""
if test "$1"  == "-g"
then
    echo running with GDB
    shift
    EXE="gdb --args"
fi
$EXE go4analysis -disable-asf -lib libGo4UserAnalysis.so $@
