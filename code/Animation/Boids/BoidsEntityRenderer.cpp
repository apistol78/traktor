/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Boids/BoidsEntity.h"
#include "Animation/Boids/BoidsEntityRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsEntityRenderer", BoidsEntityRenderer, world::IEntityRenderer)

const TypeInfoSet BoidsEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BoidsEntity >());
	return typeSet;
}

void BoidsEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (BoidsEntity* clothEntity = dynamic_type_cast< BoidsEntity* >(renderable))
		clothEntity->render(worldContext, worldRenderView, worldRenderPass);
}

void BoidsEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
