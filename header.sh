#!/usr/bin/env bash

tmp=$(mktemp)   # Create a temporary file
trap "rm -f $tmp; exit 1" 0 1 2 3 13 15

shopt -s globstar extglob nullglob
for file in **/*.@(run); do
    {
    echo "--[["
    echo " TRAKTOR"
    echo " Copyright (c) 2022 Anders Pistol."
    echo ""
    echo " This Source Code Form is subject to the terms of the Mozilla Public"
    echo " License, v. 2.0. If a copy of the MPL was not distributed with this"
    echo " file, You can obtain one at https://mozilla.org/MPL/2.0/."
    echo "]]"
    cat $file
    } > $tmp
    mv $tmp $file
done
