#include "World/WorldContext.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/ComponentEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComponentEntityRenderer", ComponentEntityRenderer, IEntityRenderer)

const TypeInfoSet ComponentEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ComponentEntity >());
	return typeSet;
}

void ComponentEntityRenderer::precull(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	Entity* entity
)
{
	ComponentEntity* gameEntity = checked_type_cast< ComponentEntity*, false >(entity);
	if (gameEntity->isVisible())
		worldContext.precull(worldRenderView, gameEntity->getEntity());
}

void ComponentEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	ComponentEntity* gameEntity = checked_type_cast< ComponentEntity*, false >(entity);
	if (gameEntity->isVisible())
		worldContext.build(worldRenderView, worldRenderPass, gameEntity->getEntity());
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
