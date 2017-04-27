/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sprite/SpriteEntity.h"
#include "Sprite/SpriteEntityRenderer.h"

namespace traktor
{
	namespace sprite
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sprite.SpriteEntityRenderer", SpriteEntityRenderer, world::IEntityRenderer)

const TypeInfoSet SpriteEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SpriteEntity >());
	return typeSet;
}

void SpriteEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
}

void SpriteEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
