#!/bin/bash

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../scripts/config.sh"

exec build/linux/ReleaseStatic/Llm.App "$@"
