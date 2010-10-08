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

Aabb PartitionMeshEntity::getBoundingBox() const
{
	return m_mesh.validate() ? m_mesh->getBoundingBox() : Aabb();
}

void PartitionMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	if (!m_mesh.validate())
		return;

	m_mesh->render(
		worldContext->getRenderContext(),
		worldRenderView,
		getTransform(worldRenderView->getInterval()),
		m_transformPrevious,
		distance,
		getUserParameter(),
		getParameterCallback()
	);

	m_first = false;
}

	}
}
