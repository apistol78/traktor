#ifndef traktor_world_IWorldCulling_H
#define traktor_world_IWorldCulling_H

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"

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

class OccluderMesh;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS IWorldCulling : public Object
{
	T_RTTI_CLASS;

public:
	virtual void placeOccluder(const OccluderMesh* mesh, const Transform& transform) = 0;

	virtual bool queryAabb(const Aabb3& aabb, const Transform& transform) const = 0;
};

	}
}

#endif	// traktor_world_IWorldCulling_H
