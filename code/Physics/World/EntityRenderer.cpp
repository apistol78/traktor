#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/EntityRenderer.h"
#include "Physics/World/RigidEntity.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.EntityRenderer", EntityRenderer, world::IEntityRenderer)

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< RigidEntity >();
	typeSet.insert< ArticulatedEntity >();
	return typeSet;
}

void EntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (RigidEntity* rigidEntity = dynamic_type_cast< RigidEntity* >(renderable))
		context.build(worldRenderView, worldRenderPass, rigidEntity->getEntity());
	else if (ArticulatedEntity* articulatedEntity = dynamic_type_cast< ArticulatedEntity* >(renderable))
	{
		const RefArray< RigidEntity >& entities = articulatedEntity->getEntities();
		for (RefArray< RigidEntity >::const_iterator i = entities.begin(); i != entities.end(); ++i)
			context.build(worldRenderView, worldRenderPass, *i);
	}
}

	}
}
