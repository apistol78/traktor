/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityRenderer.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityRenderer", GroupEntityRenderer, IEntityRenderer)

const TypeInfoSet GroupEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< GroupEntity >());
	return typeSet;
}

void GroupEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	GroupEntity* groupEntity = checked_type_cast< GroupEntity*, false >(renderable);

	const RefArray< Entity >& childEntities = groupEntity->getEntities();
	for (RefArray< Entity >::const_iterator i = childEntities.begin(); i != childEntities.end(); ++i)
		worldContext.build(worldRenderView, worldRenderPass, *i);
}

void GroupEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
}

	}
}
