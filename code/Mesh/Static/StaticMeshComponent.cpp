#include "Mesh/MeshCulling.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "World/IWorldCulling.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshComponent", StaticMeshComponent, MeshComponent)

StaticMeshComponent::StaticMeshComponent(world::Entity* owner, bool screenSpaceCulling, const resource::Proxy< StaticMesh >& mesh)
:	MeshComponent(owner, screenSpaceCulling)
,	m_mesh(mesh)
{
}

void StaticMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

Aabb3 StaticMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void StaticMeshComponent::render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass, const Transform& transform)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Aabb3 boundingBox = m_mesh->getBoundingBox();

	if (worldContext.getCulling() && !worldContext.getCulling()->queryAabb(boundingBox, transform))
		return;

	float distance = 0.0f;
	if (!isMeshVisible(
		boundingBox,
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * transform.toMatrix44(),
		worldRenderView.getProjection(),
		m_screenSpaceCulling ? 0.0001f : 0.0f,
		distance
	))
		return;

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		transform,
		distance,
		m_parameterCallback
	);
}

	}
}
