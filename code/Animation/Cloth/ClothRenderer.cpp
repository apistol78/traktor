#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothRenderer", ClothRenderer, world::IEntityRenderer)

const TypeInfoSet ClothRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ClothComponent >();
}

void ClothRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void ClothRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void ClothRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void ClothRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto clothEntity = mandatory_non_null_type_cast< ClothComponent* >(renderable);
	clothEntity->build(context, worldRenderView, worldRenderPass);
}

void ClothRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
