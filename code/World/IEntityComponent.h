#ifndef traktor_world_IEntityComponent_H
#define traktor_world_IEntityComponent_H

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
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

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

/*! \brief Entity component.
 * \ingroup World
 */
class T_DLLCLASS IEntityComponent : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void setTransform(const Transform& transform) = 0;

	virtual Aabb3 getBoundingBox() const = 0;

	virtual void update(const UpdateParams& update) = 0;

	virtual void render(WorldContext& worldContext, WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass, const Transform& transform) = 0;
};

	}
}

#endif	// traktor_world_IEntityComponent_H
