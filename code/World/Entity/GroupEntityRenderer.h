#ifndef traktor_world_GroupEntityRenderer_H
#define traktor_world_GroupEntityRenderer_H

#include "World/Entity/IEntityRenderer.h"

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

/*! \brief Group entity renderer.
 * \ingroup World
 */
class T_DLLCLASS GroupEntityRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const;

	virtual void precull(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		Entity* entity
	);

	virtual void render(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		IWorldRenderPass& worldRenderPass,
		Entity* entity
	);

	virtual void flush(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		IWorldRenderPass& worldRenderPass
	);
};

	}
}

#endif	// traktor_world_GroupEntityRenderer_H
