#!/bin/bash

export LD_LIBRARY_PATH="$HOME/.steam/bin"

echo "run" > gbd.cmd
gdb -batch -x gdb.cmd --args ./$1 -s Application.config
