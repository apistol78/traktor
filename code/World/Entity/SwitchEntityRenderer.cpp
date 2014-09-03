#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/SwitchEntity.h"
#include "World/Entity/SwitchEntityRenderer.h"

namespace traktor
{
	namespace world
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.world.SwitchEntityRenderer", SwitchEntityRenderer, IEntityRenderer)

const TypeInfoSet SwitchEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SwitchEntity >());
	return typeSet;
}

void SwitchEntityRenderer::precull(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	Entity* entity
)
{
	SwitchEntity* switchEntity = checked_type_cast< SwitchEntity*, false >(entity);
	Entity* activeEntity = switchEntity->getActiveEntity();
	if (activeEntity)
		worldContext.precull(worldRenderView, activeEntity);
}

void SwitchEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Entity* entity
)
{
	SwitchEntity* switchEntity = checked_type_cast< SwitchEntity*, false >(entity);
	Entity* activeEntity = switchEntity->getActiveEntity();
	if (activeEntity)
		worldContext.build(worldRenderView, worldRenderPass, activeEntity);
}

void SwitchEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
}

	}
}
