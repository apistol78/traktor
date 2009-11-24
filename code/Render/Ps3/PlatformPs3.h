#ifndef traktor_render_PlatformPs3_H
#define traktor_render_PlatformPs3_H

#include "Core/Config.h"

#if defined(_PS3)
#	include <cell/gcm.h>
#	include <Cg/cgc.h>
#	include <sysutil/sysutil_sysparam.h>
#else
// Assume we're compiling off-line version.
#	include <gcm_tool.h>
#	include <Cg/cgc.h>
#	include <cgutil.h>
#endif

#endif	// traktor_render_PlatformPs3_H
