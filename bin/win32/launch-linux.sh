#!/bin/bash

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:.:$HOME/.steam/bin"

chmod u+x *.so
chmod u+x $1

./$1 -s Application.config
