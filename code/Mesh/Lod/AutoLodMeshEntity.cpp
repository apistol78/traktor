#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.AutoLodMeshEntity", AutoLodMeshEntity, MeshEntity)

AutoLodMeshEntity::AutoLodMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< AutoLodMesh >& mesh)
:	MeshEntity(transform, true)
,	m_mesh(mesh)
,	m_lodDistance(0.0f)
{
}

Aabb3 AutoLodMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox(m_lodDistance);
}

bool AutoLodMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(m_lodDistance, technique);
}

void AutoLodMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	const Vector4& eyePosition = worldRenderView.getEyePosition();
	m_lodDistance = (m_transform.get().translation() - eyePosition).length();

	m_mesh->render(
		m_lodDistance,
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform,
		distance,
		getParameterCallback()
	);
}

	}
}
