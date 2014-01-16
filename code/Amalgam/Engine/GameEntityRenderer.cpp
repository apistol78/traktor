#include "Amalgam/Engine/GameEntity.h"
#include "Amalgam/Engine/GameEntityRenderer.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.GameEntityRenderer", GameEntityRenderer, world::IEntityRenderer)

const TypeInfoSet GameEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< GameEntity >());
	return typeSet;
}

void GameEntityRenderer::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
	GameEntity* gameEntity = checked_type_cast< GameEntity*, false >(entity);
	worldContext.precull(worldRenderView, gameEntity->getEntity());
}

void GameEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	GameEntity* gameEntity = checked_type_cast< GameEntity*, false >(entity);
	worldContext.build(worldRenderView, worldRenderPass, gameEntity->getEntity());
}

void GameEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
