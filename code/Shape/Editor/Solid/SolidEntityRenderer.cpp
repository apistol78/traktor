#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityRenderer.h"
#include "World/WorldBuildContext.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntityRenderer", SolidEntityRenderer, world::IEntityRenderer)

const TypeInfoSet SolidEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< SolidEntity >();
}

void SolidEntityRenderer::gather(
	const world::WorldGatherContext& context,
	const Object* renderable,
	AlignedVector< world::Light >& outLights
)
{
}

void SolidEntityRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void SolidEntityRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void SolidEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto solidEntity = dynamic_type_cast< SolidEntity* >(renderable))
	{
		solidEntity->build(
			context,
			worldRenderView,
			worldRenderPass
		);
	}
}

void SolidEntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
