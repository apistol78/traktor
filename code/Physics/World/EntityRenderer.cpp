#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/EntityRenderer.h"
#include "Physics/World/RigidEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.EntityRenderer", EntityRenderer, world::IEntityRenderer)

const TypeInfoSet EntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< RigidEntity >());
	typeSet.insert(&type_of< ArticulatedEntity >());
	return typeSet;
}

void EntityRenderer::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
	if (RigidEntity* rigidEntity = dynamic_type_cast< RigidEntity* >(entity))
		worldContext.precull(worldRenderView, rigidEntity->getEntity());
	else if (ArticulatedEntity* articulatedEntity = dynamic_type_cast< ArticulatedEntity* >(entity))
	{
		const RefArray< RigidEntity >& entities = articulatedEntity->getEntities();
		for (RefArray< RigidEntity >::const_iterator i = entities.begin(); i != entities.end(); ++i)
			worldContext.precull(worldRenderView, *i);
	}
}

void EntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	if (RigidEntity* rigidEntity = dynamic_type_cast< RigidEntity* >(entity))
		worldContext.build(worldRenderView, worldRenderPass, rigidEntity->getEntity());
	else if (ArticulatedEntity* articulatedEntity = dynamic_type_cast< ArticulatedEntity* >(entity))
	{
		const RefArray< RigidEntity >& entities = articulatedEntity->getEntities();
		for (RefArray< RigidEntity >::const_iterator i = entities.begin(); i != entities.end(); ++i)
			worldContext.build(worldRenderView, worldRenderPass, *i);
	}
}

	}
}
