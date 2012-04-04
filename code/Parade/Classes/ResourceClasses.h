#ifndef traktor_traktor_parade_ResourceClasses_H
#define traktor_traktor_parade_ResourceClasses_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptManager;

	}

	namespace parade
	{

void T_DLLCLASS registerResourceClasses(script::IScriptManager* scriptManager);

	}
}

#endif	// traktor_traktor_parade_ResourceClasses_H
