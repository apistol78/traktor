#include "Physics/World/EntityRenderer.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/ArticulatedEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.EntityRenderer", EntityRenderer, world::EntityRenderer)

const TypeSet EntityRenderer::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< RigidEntity >());
	typeSet.insert(&type_of< ArticulatedEntity >());
	return typeSet;
}

void EntityRenderer::render(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView,
	world::Entity* entity
)
{
	if (RigidEntity* rigidEntity = dynamic_type_cast< RigidEntity* >(entity))
	{
		worldContext->render(worldRenderView, rigidEntity->getEntity());
	}
	else if (ArticulatedEntity* articulatedEntity = dynamic_type_cast< ArticulatedEntity* >(entity))
	{
		const RefArray< world::SpatialEntity >& entities = articulatedEntity->getEntities();
		for (RefArray< world::SpatialEntity >::const_iterator i = entities.begin(); i != entities.end(); ++i)
			worldContext->render(worldRenderView, *i);
	}
}

	}
}
