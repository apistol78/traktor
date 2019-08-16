#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityRenderer.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupEntityRenderer", GroupEntityRenderer, IEntityRenderer)

const TypeInfoSet GroupEntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< GroupEntity >();
}

void GroupEntityRenderer::render(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	GroupEntity* groupEntity = checked_type_cast< GroupEntity*, false >(renderable);
	for (auto childEntity : groupEntity->getEntities())
		worldContext.build(worldRenderView, worldRenderPass, childEntity);
}

void GroupEntityRenderer::flush(
	WorldContext& worldContext,
	WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

	}
}
