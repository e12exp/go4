#!/bin/bash
rm -f Go4AutoSave.root # die in a fire

for l in $PWD/libgo4hack/libgo4hack.so $PWD/libfixsignals/libfixsignals.so
do
    test -f $l && export LD_PRELOAD+=:$l
done
echo "LD_PRELOAD:" $LD_PRELOAD
test -f "$1" && exec go4 "$@"
OUT=$(mktemp --suffix=.hotstart)
echo building hotstart for $i in $OUT

sed s/__STREAM__/$1/g hotstart.template >$OUT
exec go4 $OUT
