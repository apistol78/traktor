#include "Animation/Boids/BoidsEntity.h"
#include "Animation/Boids/BoidsEntityRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsEntityRenderer", BoidsEntityRenderer, world::IEntityRenderer)

const TypeInfoSet BoidsEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BoidsEntity >());
	return typeSet;
}

void BoidsEntityRenderer::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
}

void BoidsEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	if (BoidsEntity* clothEntity = dynamic_type_cast< BoidsEntity* >(entity))
		clothEntity->render(worldContext, worldRenderView, worldRenderPass);
}

void BoidsEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
