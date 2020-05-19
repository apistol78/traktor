#include "Mesh/MeshCulling.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

static const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshComponent", StaticMeshComponent, MeshComponent)

StaticMeshComponent::StaticMeshComponent(const resource::Proxy< StaticMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
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

void StaticMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Transform worldTransform = m_transform.get(worldRenderView.getInterval());
	Transform lastWorldTransform = m_transform.get(worldRenderView.getInterval() - 1.0f);

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (worldTransform == lastWorldTransform)
			return;
	}

	float distance = 0.0f;
	if (!isMeshVisible(
		m_mesh->getBoundingBox(),
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * worldTransform.toMatrix44(),
		worldRenderView.getProjection(),
		m_screenSpaceCulling ? 0.0001f : 0.0f,
		distance
	))
		return;

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		lastWorldTransform,
		worldTransform,
		distance,
		m_parameterCallback
	);
}

	}
}
