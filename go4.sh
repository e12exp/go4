#!/bin/bash
make -C libgo4hack && LD_PRELOAD=$PWD/libgo4hack/libgo4hack.so go4 "$@"
