#ifndef traktor_animation_PathEntityRenderer_H
#define traktor_animation_PathEntityRenderer_H

#include "World/Entity/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Movement path entity renderer.
 * \ingroup Animation
 */
class T_DLLCLASS PathEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const;

	virtual void precull(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::Entity* entity
	);

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);
};

	}
}

#endif	// traktor_animation_PathEntityRenderer_H
