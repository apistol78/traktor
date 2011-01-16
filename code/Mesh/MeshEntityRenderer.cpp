#include "Mesh/MeshCulling.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/MeshEntityRenderer.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntityRenderer", MeshEntityRenderer, world::IEntityRenderer)

const TypeInfoSet MeshEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshEntity >());
	return typeSet;
}

void MeshEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	MeshEntity* meshEntity = checked_type_cast< MeshEntity* >(entity);
	Aabb3 boundingBox = meshEntity->getBoundingBox();
	Transform transform; meshEntity->getTransform(transform);

	float distance = 0.0f;
	if (!isMeshVisible(
		boundingBox,
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * transform.toMatrix44(),
		worldRenderView.getProjection(),
		1e-4f,
		distance
	))
		return;

	meshEntity->render(
		worldContext,
		worldRenderView,
		worldRenderPass,
		distance
	);
}

void MeshEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
