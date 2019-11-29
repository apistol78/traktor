#include "World/WorldContext.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityRenderer", GroupEntityRenderer, IEntityRenderer)

GroupEntityRenderer::GroupEntityRenderer(uint32_t filter)
:	m_filter(filter)
{
}

const TypeInfoSet GroupEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet<
		GroupComponent,
		GroupEntity
	>();
}

void GroupEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto groupComponent = dynamic_type_cast< GroupComponent* >(renderable))
	{
		for (auto childEntity : groupComponent->getEntities())
			worldContext.build(worldRenderView, worldRenderPass, childEntity);
	}
	else if (auto groupEntity = dynamic_type_cast< GroupEntity* >(renderable))
	{
		if ((groupEntity->getMask() & m_filter) != 0)
		{
			for (auto childEntity : groupEntity->getEntities())
				worldContext.build(worldRenderView, worldRenderPass, childEntity);
		}
	}
}

void GroupEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Entity* rootEntity
)
{
}

	}
}
