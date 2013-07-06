#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMeshEntity", IndoorMeshEntity, MeshEntity)

IndoorMeshEntity::IndoorMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< IndoorMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
{
}

Aabb3 IndoorMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool IndoorMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void IndoorMeshEntity::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView
)
{
}

void IndoorMeshEntity::render(
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
		getTransform(worldRenderView.getInterval()),
		distance,
		getParameterCallback()
	);
}

	}
}
