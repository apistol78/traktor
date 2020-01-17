#pragma once

#include "World/IEntityRenderer.h"

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

/*! Movement path entity renderer.
 * \ingroup Animation
 */
class T_DLLCLASS PathEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const Object* renderable,
		AlignedVector< world::Light >& outLights
	) override final;

	virtual void build(
		world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

	virtual void flush(world::WorldContext& worldContext) override final;
};

	}
}

