#include "World/IEntityComponent.h"
#include "World/WorldContext.h"
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
	const WorldContext& worldContext,
	const Object* renderable,
	AlignedVector< Light >& outLights
)
{
	const ComponentEntity* componentEntity = mandatory_non_null_type_cast< const ComponentEntity* >(renderable);
	for (auto component : componentEntity->getComponents())
		worldContext.gather(component, outLights);
}

void ComponentEntityRenderer::build(
	const WorldContext& worldContext,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	ComponentEntity* componentEntity = mandatory_non_null_type_cast< ComponentEntity* >(renderable);
	for (auto component : componentEntity->getComponents())
		worldContext.build(worldRenderView, worldRenderPass, component);
}

void ComponentEntityRenderer::flush(
	const WorldContext& worldContext,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

void ComponentEntityRenderer::flush(const WorldContext& worldContext)
{
}

	}
}
