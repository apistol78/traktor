#include "World/Entity/TransientEntity.h"
#include "World/Entity/TransientEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.TransientEntityRenderer", TransientEntityRenderer, IEntityRenderer)

const TypeInfoSet TransientEntityRenderer::getEntityTypes() const
{
	TypeInfoSet TypeInfoSet;
	TypeInfoSet.insert(&type_of< TransientEntity >());
	return TypeInfoSet;
}

void TransientEntityRenderer::precull(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	Entity* entity
)
{
	if (TransientEntity* transientEntity = checked_type_cast< TransientEntity* >(entity))
		transientEntity->precull(worldContext, worldRenderView);
}

void TransientEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	if (TransientEntity* transientEntity = checked_type_cast< TransientEntity* >(entity))
		transientEntity->render(worldContext, worldRenderView, worldRenderPass);
}

void TransientEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
}

	}
}
