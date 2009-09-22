#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Static/StaticMesh.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshEntity", StaticMeshEntity, MeshEntity)

StaticMeshEntity::StaticMeshEntity(const Transform& transform, const resource::Proxy< StaticMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
{
}

Aabb StaticMeshEntity::getBoundingBox() const
{
	return m_mesh.validate() ? m_mesh->getBoundingBox() : Aabb();
}

void StaticMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	if (!m_mesh.validate())
		return;

	m_mesh->render(
		worldContext->getRenderContext(),
		worldRenderView,
		m_transform,
		m_transformPrevious,
		distance,
		getUserParameter(),
		getParameterCallback()
	);

	m_transformPrevious = m_transform;
}

void StaticMeshEntity::update(const world::EntityUpdate* update)
{
}

	}
}
