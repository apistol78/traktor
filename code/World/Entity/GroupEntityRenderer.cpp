#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityRenderer.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityRenderer", GroupEntityRenderer, IEntityRenderer)

const TypeInfoSet GroupEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< GroupEntity >());
	return typeSet;
}

void GroupEntityRenderer::precull(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	Entity* entity
)
{
	GroupEntity* groupEntity = checked_type_cast< GroupEntity*, false >(entity);

	const RefArray< Entity >& childEntities = groupEntity->getEntities();
	for (RefArray< Entity >::const_iterator i = childEntities.begin(); i != childEntities.end(); ++i)
		worldContext.precull(worldRenderView, *i);
}

void GroupEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	GroupEntity* groupEntity = checked_type_cast< GroupEntity*, false >(entity);

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
