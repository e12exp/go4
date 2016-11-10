#!/bin/bash

anaPIDs()
{
    ps -AHF | grep go[4]ana | grep -v "sh -c" | cut -c 8-15
}

makebuild()
{
    rm -f debug.o
    CXXFLAGS=$1 make debug.o # works only after fixing $GO4SYS/Makefile.config
    make
}

test -z "$1" && echo "usage: $0 hotstartfile" && exit 0
anaPIDs | xargs -r kill
sleep 1
ulimit -c unlimited
LD_PRELOAD=$PWD/libfixsignals/libfixsignals.so go4 "$@" &
makebuild -DDBG_GDB

while test -z "$(anaPIDs)"
do
    echo waiting...
    sleep 10
done
PID=$(anaPIDs)
echo attaching to pid $PID
gdb --pid $PID
killall go4
makebuild
