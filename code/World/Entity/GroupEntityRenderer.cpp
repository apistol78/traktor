#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/SpatialGroupEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityRenderer", GroupEntityRenderer, IEntityRenderer)

const TypeSet GroupEntityRenderer::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< GroupEntity >());
	typeSet.insert(&type_of< SpatialGroupEntity >());
	return typeSet;
}

void GroupEntityRenderer::render(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView,
	world::Entity* entity
)
{
	if (GroupEntity* groupEntity = dynamic_type_cast< GroupEntity* >(entity))
	{
		const RefArray< Entity >& childEntities = groupEntity->getEntities();
		for (RefArray< Entity >::const_iterator i = childEntities.begin(); i != childEntities.end(); ++i)
			worldContext->build(worldRenderView, *i);
	}
	else if (SpatialGroupEntity* spatialGroupEntity = dynamic_type_cast< SpatialGroupEntity* >(entity))
	{
		const RefArray< SpatialEntity >& childEntities = spatialGroupEntity->getEntities();
		for (RefArray< SpatialEntity >::const_iterator i = childEntities.begin(); i != childEntities.end(); ++i)
			worldContext->build(worldRenderView, *i);
	}
}

void GroupEntityRenderer::flush(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView
)
{
}

	}
}
