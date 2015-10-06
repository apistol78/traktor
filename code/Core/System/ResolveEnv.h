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

/*! \brief Resolve all environment variable references in input string.
 */
std::wstring T_DLLCLASS resolveEnv(const std::wstring& s, const OS::envmap_t* envmap);

}

#endif	// traktor_ResolveEnv_H
