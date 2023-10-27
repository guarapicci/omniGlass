#!/bin/sh

export LD_LIBRARY_PATH="./lib/"
export OMNIGLASS_BIN_SUBFOLDER="./bin/"

[ -n $1 ] && cd $OMNIGLASS_BIN_SUBFOLDER && ./$1
