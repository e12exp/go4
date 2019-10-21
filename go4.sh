#!/bin/bash
rm -f Go4AutoSave.root # die in a fire
export LD_PRELOAD=$PWD/libgo4hack/libgo4hack.so:libfixsignals/libfixsignals.so
test -f "$1" && exec go4 "$@"
OUT=$(mktemp --suffix=.hotstart)
echo building hotstart for $i in $OUT

sed s/__STREAM__/$1/g hotstart.template >$OUT
exec go4 $OUT
