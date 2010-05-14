#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMeshEntity", IndoorMeshEntity, MeshEntity)

IndoorMeshEntity::IndoorMeshEntity(const Transform& transform, const resource::Proxy< IndoorMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
{
}

Aabb IndoorMeshEntity::getBoundingBox() const
{
	return m_mesh.validate() ? m_mesh->getBoundingBox() : Aabb();
}

void IndoorMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	if (!m_mesh.validate())
		return;

	m_mesh->render(
		worldContext->getRenderContext(),
		worldRenderView,
		m_transform,
		distance,
		getParameterCallback()
	);

	m_first = false;
}

	}
}
