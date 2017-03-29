#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.PartitionMeshEntity", PartitionMeshEntity, MeshEntity)

PartitionMeshEntity::PartitionMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< PartitionMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
{
}

Aabb3 PartitionMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool PartitionMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void PartitionMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderView,
		worldRenderPass,
		m_transform.get(worldRenderView.getInterval()),
		distance,
		getParameterCallback()
	);
}

	}
}
