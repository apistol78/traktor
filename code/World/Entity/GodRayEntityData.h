#ifndef traktor_world_GodRayEntityData_H
#define traktor_world_GodRayEntityData_H

#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief God-ray entity data.
 * \ingroup World
 */
class T_DLLCLASS GodRayEntityData : public EntityData
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_world_GodRayEntityData_H
