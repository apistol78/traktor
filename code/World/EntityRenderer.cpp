#include "World/Entity.h"
#include "World/EntityRenderer.h"
#include "World/IEntityComponent.h"
#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"
#include "World/WorldSetupContext.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityRenderer", EntityRenderer, IEntityRenderer)

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< Entity >();
}

void EntityRenderer::gather(
	const WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< Light >& outLights
)
{
	const Entity* entity = mandatory_non_null_type_cast< const Entity* >(renderable);
	for (auto component : entity->getComponents())
		context.gather(component, outLights);
}

void EntityRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
	Entity* entity = mandatory_non_null_type_cast< Entity* >(renderable);
	for (auto component : entity->getComponents())
		context.setup(worldRenderView, component);
}

void EntityRenderer::setup(
	const WorldSetupContext& context
)
{
}

void EntityRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	Entity* entity = mandatory_non_null_type_cast< Entity* >(renderable);
	for (auto component : entity->getComponents())
		context.build(worldRenderView, worldRenderPass, component);
}

void EntityRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

	}
}
