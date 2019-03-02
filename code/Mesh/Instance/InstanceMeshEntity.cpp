#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshEntity", InstanceMeshEntity, MeshEntity)

InstanceMeshEntity::InstanceMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< InstanceMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
{
}

Aabb3 InstanceMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool InstanceMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void InstanceMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	T_ASSERT_M (0, L"Forgot to register InstanceMeshEntityRenderer?");
}

	}
}
