#!/bin/bash
OUTPUT=$(readlink -f "$1" )
shift
. /u/land/fake_cvmfs/sourceme.sh ""
. go4login


echo $OUTPUT | grep -q ".root" || { echo "first argument is not a root output file" ; exit 1; }

IN="$@"
OLDPWD=$PWD

cd $(dirname $0)

exec go4analysis $IN  -enable-asf 30 -asf $OUTPUT -rate

