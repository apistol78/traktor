#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../config.sh"

codeblocks 'build/linux/SolutionBuilder.workspace'
