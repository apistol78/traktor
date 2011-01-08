#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/SpatialGroupEntity.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityRenderer", GroupEntityRenderer, IEntityRenderer)

const TypeInfoSet GroupEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< GroupEntity >());
	typeSet.insert(&type_of< SpatialGroupEntity >());
	return typeSet;
}

void GroupEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	if (GroupEntity* groupEntity = dynamic_type_cast< GroupEntity* >(entity))
	{
		const RefArray< Entity >& childEntities = groupEntity->getEntities();
		for (RefArray< Entity >::const_iterator i = childEntities.begin(); i != childEntities.end(); ++i)
			worldContext.build(worldRenderView, worldRenderPass, *i);
	}
	else if (SpatialGroupEntity* spatialGroupEntity = dynamic_type_cast< SpatialGroupEntity* >(entity))
	{
		const RefArray< SpatialEntity >& childEntities = spatialGroupEntity->getEntities();
		for (RefArray< SpatialEntity >::const_iterator i = childEntities.begin(); i != childEntities.end(); ++i)
			worldContext.build(worldRenderView, worldRenderPass, *i);
	}
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
