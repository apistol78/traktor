#include "Animation/AnimationEntityRenderer.h"
#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Cloth/ClothComponent.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationEntityRenderer", AnimationEntityRenderer, world::IEntityRenderer)

const TypeInfoSet AnimationEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< BoidsComponent >();
	typeSet.insert< ClothComponent >();
	return typeSet;
}

void AnimationEntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void AnimationEntityRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void AnimationEntityRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void AnimationEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto boidsComponent = dynamic_type_cast< BoidsComponent* >(renderable))
		boidsComponent->build(context, worldRenderView, worldRenderPass);
	else if (auto clothEntity = dynamic_type_cast< ClothComponent* >(renderable))
		clothEntity->build(context, worldRenderView, worldRenderPass);
}

void AnimationEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
