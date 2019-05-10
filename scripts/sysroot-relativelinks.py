#!/usr/bin/env python
import sys
import os

# source: https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py

# Take a sysroot directory and turn all the absolute symlinks and turn them into
# relative ones such that the sysroot is usable within another system.

if len(sys.argv) != 2:
    print("Usage is " + sys.argv[0] + "<directory>")
    sys.exit(1)

topdir = sys.argv[1]
topdir = os.path.abspath(topdir)

def handlelink(filePath, subdir):
    link = os.readlink(filePath)
    if link[0] != "/":
        return
    if link.startswith(topdir):
        return
    
    newLink = os.path.relpath(topdir+link, subdir)

    print("\t%s replacing %s => %s" % (filePath, link, newLink))
    os.unlink(filePath)
    os.symlink(newLink, filePath)

for subdir, dirs, files in os.walk(topdir):
    for file in files:
        filePath = os.path.join(subdir, file)
        if os.path.islink(filePath):
            handlelink(filePath, subdir)
