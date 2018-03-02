#include "Shared/EntityRenderer.h"
#include "Shared/QuadComponent.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"EntityRenderer", EntityRenderer, world::IEntityRenderer)

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< QuadComponent >());
	return typeSet;
}

void EntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	QuadComponent* quadComponent = mandatory_non_null_type_cast< QuadComponent* >(renderable);
	quadComponent->render(worldContext.getRenderContext(), worldRenderView, worldRenderPass);
}

void EntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}
