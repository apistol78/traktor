#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/SpatialGroupEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityRenderer", GroupEntityRenderer, EntityRenderer)

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
	else if (SpatialGroupEntity* groupEntity = dynamic_type_cast< SpatialGroupEntity* >(entity))
	{
		// Cull entire group, early out rendering of all child entities.
		Aabb boundingBox = groupEntity->getWorldBoundingBox();
		if (boundingBox.empty())
			return;

		Vector4 center = worldRenderView->getView() * boundingBox.getCenter();
		Scalar radius = boundingBox.getExtent().length();

		if (!worldRenderView->getCullFrustum().inside(center, radius))
			return;

		const RefArray< SpatialEntity >& childEntities = groupEntity->getEntities();
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
