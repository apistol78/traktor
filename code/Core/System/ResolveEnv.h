#pragma once

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

/*! Resolve all environment variable references in input string.
 */
std::wstring T_DLLCLASS resolveEnv(const std::wstring& s, const Environment* env);

}

