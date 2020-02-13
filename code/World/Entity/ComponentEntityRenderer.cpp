#include "World/IEntityComponent.h"
#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/ComponentEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComponentEntityRenderer", ComponentEntityRenderer, IEntityRenderer)

const TypeInfoSet ComponentEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ComponentEntity >();
}

void ComponentEntityRenderer::gather(
	const WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< Light >& outLights
)
{
	const ComponentEntity* componentEntity = mandatory_non_null_type_cast< const ComponentEntity* >(renderable);
	for (auto component : componentEntity->getComponents())
		context.gather(component, outLights);
}

void ComponentEntityRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	ComponentEntity* componentEntity = mandatory_non_null_type_cast< ComponentEntity* >(renderable);
	for (auto component : componentEntity->getComponents())
		context.build(worldRenderView, worldRenderPass, component);
}

void ComponentEntityRenderer::flush(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

void ComponentEntityRenderer::flush(const WorldBuildContext& context)
{
}

	}
}
