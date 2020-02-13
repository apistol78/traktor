#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"
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
	const WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< Light >& outLights
)
{
	if (auto groupComponent = dynamic_type_cast< const GroupComponent* >(renderable))
	{
		for (auto childEntity : groupComponent->getEntities())
			context.gather(childEntity, outLights);
	}
	else if (auto groupEntity = dynamic_type_cast< const GroupEntity* >(renderable))
	{
		if ((groupEntity->getMask() & m_filter) != 0)
		{
			for (auto childEntity : groupEntity->getEntities())
				context.gather(childEntity, outLights);
		}
	}
}

void GroupEntityRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto groupComponent = dynamic_type_cast< GroupComponent* >(renderable))
	{
		for (auto childEntity : groupComponent->getEntities())
			context.build(worldRenderView, worldRenderPass, childEntity);
	}
	else if (auto groupEntity = dynamic_type_cast< GroupEntity* >(renderable))
	{
		if ((groupEntity->getMask() & m_filter) != 0)
		{
			for (auto childEntity : groupEntity->getEntities())
				context.build(worldRenderView, worldRenderPass, childEntity);
		}
	}
}

void GroupEntityRenderer::flush(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

void GroupEntityRenderer::flush(const WorldBuildContext& context)
{
}

	}
}
