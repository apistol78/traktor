#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshEntity", InstanceMeshEntity, MeshEntity)

InstanceMeshEntity::InstanceMeshEntity(const Transform& transform, const resource::Proxy< InstanceMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
{
}

Aabb InstanceMeshEntity::getBoundingBox() const
{
	return m_mesh.validate() ? m_mesh->getBoundingBox() : Aabb();
}

void InstanceMeshEntity::update(const world::EntityUpdate* update)
{
	MeshEntity::update(update);
	m_mesh.validate();
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
