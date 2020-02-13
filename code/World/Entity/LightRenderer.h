#pragma once

#include "World/IEntityRenderer.h"

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

/*! Light entity renderer.
 * \ingroup World
 */
class T_DLLCLASS LightRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void gather(
		const WorldContext& worldContext,
		const Object* renderable,
		AlignedVector< Light >& outLights
	) override final;

	virtual void build(
		const WorldContext& worldContext,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		const WorldContext& worldContext,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;

	virtual void flush(const WorldContext& worldContext) override final;
};

	}
}

