#pragma once

#include "World/IEntityRenderer.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! Physics entity renderer.
 * \ingroup Physics
 */
class T_DLLCLASS EntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const world::WorldContext& worldContext,
		const Object* renderable,
		AlignedVector< world::Light >& outLights
	) override final
	{ /* Not implemented */ }

	virtual void build(
		const world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		const world::WorldContext& worldContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final
	{ /* Not implemented */ }

	virtual void flush(const world::WorldContext& worldContext) override final
	{ /* Not implemented */ }
};

	}
}

