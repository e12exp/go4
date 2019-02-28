#!/bin/bash
OUT=$(mktemp --suffix=.hotstart)
echo building hotstart for $i in $OUT

sed s/__STREAM__/$1/g template.hotstart >$OUT
export LD_PRELOAD=$PWD/libgo4hack/libgo4hack.so:libfixsignals/libfixsignals.so
exec go4 $OUT
