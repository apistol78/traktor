#pragma once

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace spray
	{

class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! Effect entity renderer.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EffectEntityRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance);

	void setLodDistances(float lod1Distance, float lod2Distance);

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

private:
	Ref< PointRenderer > m_pointRenderer;
	Ref< MeshRenderer > m_meshRenderer;
	Ref< TrailRenderer > m_trailRenderer;
};

	}
}

