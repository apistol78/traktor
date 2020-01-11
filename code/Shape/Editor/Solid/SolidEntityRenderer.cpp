#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityRenderer.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntityRenderer", SolidEntityRenderer, world::IEntityRenderer)

const TypeInfoSet SolidEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< SolidEntity >();
}

void SolidEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto solidEntity = dynamic_type_cast< SolidEntity* >(renderable))
	{
		solidEntity->render(
			worldContext,
			worldRenderView,
			worldRenderPass
		);
	}
}

void SolidEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	world::Entity* rootEntity
)
{
}

void SolidEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::Entity* rootEntity
)
{
}

	}
}
