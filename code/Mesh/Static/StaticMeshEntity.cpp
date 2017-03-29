#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshEntity", StaticMeshEntity, MeshEntity)

StaticMeshEntity::StaticMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< StaticMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
{
}

Aabb3 StaticMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool StaticMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void StaticMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform.get0(),
		m_transform.get(worldRenderView.getInterval()),
		distance,
		getParameterCallback()
	);
}

	}
}
