#ifndef traktor_traktor_amalgam_PhysicsClasses_H
#define traktor_traktor_amalgam_PhysicsClasses_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
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

	namespace amalgam
	{

void T_DLLCLASS registerPhysicsClasses(script::IScriptManager* scriptManager);

	}
}

#endif	// traktor_traktor_amalgam_PhysicsClasses_H
