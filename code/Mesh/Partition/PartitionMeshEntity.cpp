#include "Mesh/Partition/PartitionMeshEntity.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.PartitionMeshEntity", PartitionMeshEntity, MeshEntity)

PartitionMeshEntity::PartitionMeshEntity(const Transform& transform, const resource::Proxy< PartitionMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
{
}

Aabb3 PartitionMeshEntity::getBoundingBox() const
{
	return m_mesh.validate() ? m_mesh->getBoundingBox() : Aabb3();
}

bool PartitionMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh.validate() ? m_mesh->supportTechnique(technique) : false;
}

void PartitionMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	if (!m_mesh.validate())
		return;

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderView,
		worldRenderPass,
		getTransform(worldRenderView.getInterval()),
		distance,
		getUserParameter(),
		getParameterCallback()
	);

	m_first = false;
}

	}
}
