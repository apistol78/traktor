#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/GroupEntity.h"
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
	return typeSet;
}

void GroupEntityRenderer::render(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView,
	world::Entity* entity
)
{
	GroupEntity* groupEntity = checked_type_cast< GroupEntity* >(entity);
	const RefArray< Entity >& childEntities = groupEntity->getEntities();
	for (RefArray< Entity >::const_iterator i = childEntities.begin(); i != childEntities.end(); ++i)
		worldContext->build(worldRenderView, *i);
}

void GroupEntityRenderer::flush(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView
)
{
}

	}
}
