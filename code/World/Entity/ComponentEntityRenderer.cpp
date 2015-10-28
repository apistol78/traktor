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
}

void ComponentEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	ComponentEntity* componentEntity = checked_type_cast< ComponentEntity*, false >(entity);
	componentEntity->render(worldContext, worldRenderView, worldRenderPass);
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
