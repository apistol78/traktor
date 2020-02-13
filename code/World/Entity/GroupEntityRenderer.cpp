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

void GroupEntityRenderer::gather(
	const WorldContext& worldContext,
	const Object* renderable,
	AlignedVector< Light >& outLights
)
{
	if (auto groupComponent = dynamic_type_cast< const GroupComponent* >(renderable))
	{
		for (auto childEntity : groupComponent->getEntities())
			worldContext.gather(childEntity, outLights);
	}
	else if (auto groupEntity = dynamic_type_cast< const GroupEntity* >(renderable))
	{
		if ((groupEntity->getMask() & m_filter) != 0)
		{
			for (auto childEntity : groupEntity->getEntities())
				worldContext.gather(childEntity, outLights);
		}
	}
}

void GroupEntityRenderer::build(
	const WorldContext& worldContext,
	const WorldRenderView& worldRenderView,
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
	const WorldContext& worldContext,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

void GroupEntityRenderer::flush(const WorldContext& worldContext)
{
}

	}
}
