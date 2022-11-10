#include "Mesh/MeshCulling.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMeshComponent", IndoorMeshComponent, MeshComponent)

IndoorMeshComponent::IndoorMeshComponent(const resource::Proxy< IndoorMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
{
}

void IndoorMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

Aabb3 IndoorMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void IndoorMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Transform transform = m_transform.get(worldRenderView.getInterval());
	Aabb3 boundingBox = m_mesh->getBoundingBox();

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

	m_mesh->build(
		context.getRenderContext(),
		worldRenderView,
		worldRenderPass,
		transform,
		distance,
		m_parameterCallback
	);
}

}
