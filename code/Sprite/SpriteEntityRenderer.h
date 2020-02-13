#pragma once

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRITE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sprite
	{

/*! Sprite entity renderer.
 * \ingroup Sprite
 */
class T_DLLCLASS SpriteEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual void gather(
		const world::WorldContext& worldContext,
		const Object* renderable,
		AlignedVector< world::Light >& outLights
	) override final;

	virtual void build(
		const world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		world::Entity* entity
	) override final;

	virtual void flush(
		const world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;
};

	}
}

