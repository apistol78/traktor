#!/bin/bash
set -e

# Source environment configuration.
. "`dirname \"$BASH_SOURCE\"`/../../scripts/config.sh"

# Build Webber and the engine modules it depends on; default target is ReleaseShared.
$TRAKTOR_HOME/bin/linux/releasestatic/Traktor.SolutionBuilder.App \
	-f=direct \
	--dependent-only \
	--target=${1:-ReleaseShared} \
	$TRAKTOR_HOME/tools/Webber/WebberLinux.xms
