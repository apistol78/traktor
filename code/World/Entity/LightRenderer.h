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

	virtual void render(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Entity* rootEntity
	) override final;
};

	}
}

