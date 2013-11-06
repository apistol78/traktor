#!/bin/bash

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:.:$HOME/.steam/bin"

chmod u+x *.so
chmod u+x $1

gdb -ex=r --args ./$1 -s Application.config
