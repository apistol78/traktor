/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_PlatformPs3_H
#define traktor_render_PlatformPs3_H

#if defined(_PS3)
#	include <cell/gcm.h>
#	include <Cg/cgc.h>
#	include <sysutil/sysutil_sysparam.h>
#else
// Assume we're compiling off-line version.
#	include <gcm_tool.h>
#	include <Cg/cgc.h>
#	include <cgutil.h>
#	define T_HAVE_TYPES

#	define T_SCE_PERF_MEASURE 1

#	if T_SCE_PERF_MEASURE
#		define uint32_t uint32_t
#		define uint64_t uint64_t
#		include <SCEShaderPerf.h>
#		undef uint32_t
#		undef uint64_t
#	endif

#endif

#include "Render/Ps3/ConfigPs3.h"

//#define T_GCM_CALL(fnc) fnc
//#define T_GCM_CALL(fnc) fnc##Inline
#define T_GCM_CALL(fnc) fnc##UnsafeInline

#endif	// traktor_render_PlatformPs3_H
