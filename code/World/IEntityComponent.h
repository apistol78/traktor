#ifndef traktor_world_IEntityComponent_H
#define traktor_world_IEntityComponent_H

#include "Core/Object.h"
#include "World/WorldTypes.h"

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

/*! \brief Entity component.
 * \ingroup World
 */
class T_DLLCLASS IEntityComponent : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update(const UpdateParams& update) = 0;
};

	}
}

#endif	// traktor_world_IEntityComponent_H
