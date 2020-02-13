#include "Sprite/SpriteEntity.h"
#include "Sprite/SpriteEntityRenderer.h"

namespace traktor
{
	namespace sprite
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sprite.SpriteEntityRenderer", SpriteEntityRenderer, world::IEntityRenderer)

const TypeInfoSet SpriteEntityRenderer::getEntityTypes() const
{
	return makeTypeInfoSet< SpriteEntity >();
}

void SpriteEntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void SpriteEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
}

void SpriteEntityRenderer::flush(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
