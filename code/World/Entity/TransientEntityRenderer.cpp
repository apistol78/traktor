#include "World/Entity/TransientEntity.h"
#include "World/Entity/TransientEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.TransientEntityRenderer", TransientEntityRenderer, IEntityRenderer)

const TypeInfoSet TransientEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet TypeInfoSet;
	TypeInfoSet.insert(&type_of< TransientEntity >());
	return TypeInfoSet;
}

void TransientEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (TransientEntity* transientEntity = checked_type_cast< TransientEntity* >(renderable))
		transientEntity->render(worldContext, worldRenderView, worldRenderPass);
}

void TransientEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	IWorldRenderPass& worldRenderPass
)
{
}

	}
}
