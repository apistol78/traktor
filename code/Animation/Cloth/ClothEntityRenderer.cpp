#include "Animation/Cloth/ClothEntity.h"
#include "Animation/Cloth/ClothEntityRenderer.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothEntityRenderer", ClothEntityRenderer, world::IEntityRenderer)

const TypeInfoSet ClothEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ClothEntity >();
}

void ClothEntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void ClothEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	ClothEntity* clothEntity = mandatory_non_null_type_cast< ClothEntity* >(renderable);
	clothEntity->build(context, worldRenderView, worldRenderPass);
}

void ClothEntityRenderer::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

void ClothEntityRenderer::flush(const world::WorldBuildContext& context)
{
}

	}
}
