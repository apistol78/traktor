#include "Mesh/MeshCulling.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{
	namespace
	{

static const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshComponent", StaticMeshComponent, MeshComponent)

StaticMeshComponent::StaticMeshComponent(const resource::Proxy< StaticMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_lastTransform(Transform::identity())
{
}

void StaticMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

void StaticMeshComponent::setOwner(world::Entity* owner)
{
	if (owner != nullptr)
		m_lastTransform = owner->getTransform();
	MeshComponent::setOwner(owner);
}

Aabb3 StaticMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void StaticMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	const StaticMesh::techniqueParts_t* techniqueParts = m_mesh->findTechniqueParts(worldRenderPass.getTechnique());
	if (!techniqueParts)
		return;

	Transform worldTransform = m_transform.get(worldRenderView.getInterval());

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (worldTransform == m_lastTransform)
			return;
	}

	float distance = 0.0f;
	if (!isMeshVisible(
		m_mesh->getBoundingBox(),
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * worldTransform.toMatrix44(),
		worldRenderView.getProjection(),
		m_screenSpaceCulling ? 0.001f : 0.0f,
		distance
	))
		return;

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		*techniqueParts,
		m_lastTransform,
		worldTransform,
		distance,
		m_parameterCallback
	);

	// Save last rendered transform so we can properly write velocities next frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
		m_lastTransform = worldTransform;
}

}
