/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/WorldContext.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/ComponentEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComponentEntityRenderer", ComponentEntityRenderer, IEntityRenderer)

const TypeInfoSet ComponentEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ComponentEntity >());
	return typeSet;
}

void ComponentEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	ComponentEntity* componentEntity = checked_type_cast< ComponentEntity*, false >(renderable);
	componentEntity->render(worldContext, worldRenderView, worldRenderPass);
}

void ComponentEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
}

	}
}
