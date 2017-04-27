/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ResolveEnv_H
#define traktor_ResolveEnv_H

#include "Core/System/OS.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Environment;

/*! \brief Resolve all environment variable references in input string.
 */
std::wstring T_DLLCLASS resolveEnv(const std::wstring& s, const Environment* env);

}

#endif	// traktor_ResolveEnv_H
