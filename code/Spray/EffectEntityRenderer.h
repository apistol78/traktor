/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_EffectEntityRenderer_H
#define traktor_spray_EffectEntityRenderer_H

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

class EffectEntity;
class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! \brief Effect entity renderer.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EffectEntityRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance);

	void setLodDistances(float lod1Distance, float lod2Distance);

	virtual const TypeInfoSet getRenderableTypes() const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) T_OVERRIDE T_FINAL;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) T_OVERRIDE T_FINAL;

private:
	Ref< PointRenderer > m_pointRenderer;
	Ref< MeshRenderer > m_meshRenderer;
	Ref< TrailRenderer > m_trailRenderer;
};

	}
}

#endif	// traktor_spray_EffectEntityRenderer_H
