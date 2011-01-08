#include "Animation/PathEntity/PathEntity.h"
#include "Animation/PathEntity/PathEntityRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityRenderer", PathEntityRenderer, world::IEntityRenderer)

const TypeInfoSet PathEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PathEntity >());
	return typeSet;
}

void PathEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	if (PathEntity* pathEntity = dynamic_type_cast< PathEntity* >(entity))
		pathEntity->render(worldContext, worldRenderView, worldRenderPass);
}

void PathEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
