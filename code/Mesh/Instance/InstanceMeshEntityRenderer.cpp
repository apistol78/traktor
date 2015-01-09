#include "Mesh/Instance/InstanceMeshEntityRenderer.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/MeshCulling.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshEntityRenderer", InstanceMeshEntityRenderer, world::IEntityRenderer)

const TypeInfoSet InstanceMeshEntityRenderer::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< InstanceMeshEntity >());
	return typeSet;
}

void InstanceMeshEntityRenderer::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::Entity* entity
)
{
	InstanceMeshEntity* meshEntity = checked_type_cast< InstanceMeshEntity*, false >(entity);
	meshEntity->precull(worldContext, worldRenderView);
}

void InstanceMeshEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	InstanceMeshEntity* meshEntity = checked_type_cast< InstanceMeshEntity* >(entity);
	T_ASSERT_M (!meshEntity->getParameterCallback(), L"Instance mesh entities doesn't support parameter callback");
	
	InstanceMesh* mesh = meshEntity->getMesh();

	if (!mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Aabb3 boundingBox = meshEntity->getBoundingBox();
	Transform transform = meshEntity->getTransform(worldRenderView.getInterval());

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

	m_meshInstances[mesh].push_back(std::make_pair(
		packInstanceMeshData(transform),
		distance
	));
}

void InstanceMeshEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	for (SmallMap< InstanceMesh*, AlignedVector< InstanceMesh::instance_distance_t > >::iterator i = m_meshInstances.begin(); i != m_meshInstances.end(); ++i)
	{
		if (i->second.empty())
			continue;

		T_ASSERT (i->first);
		i->first->render(
			worldContext.getRenderContext(),
			worldRenderPass,
			i->second,
			0
		);

		i->second.resize(0);
	}
}

	}
}
