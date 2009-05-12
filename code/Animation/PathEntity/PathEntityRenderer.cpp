#include "Animation/PathEntity/PathEntityRenderer.h"
#include "Animation/PathEntity/PathEntity.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityRenderer", PathEntityRenderer, world::EntityRenderer)

const TypeSet PathEntityRenderer::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< PathEntity >());
	return typeSet;
}

void PathEntityRenderer::render(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView,
	world::Entity* entity
)
{
	if (PathEntity* pathEntity = dynamic_type_cast< PathEntity* >(entity))
		pathEntity->render(worldContext, worldRenderView);
}

void PathEntityRenderer::flush(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView
)
{
}

	}
}
