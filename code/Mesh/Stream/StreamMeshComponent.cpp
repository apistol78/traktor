#include "Mesh/MeshCulling.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StreamMeshComponent", StreamMeshComponent, MeshComponent)

StreamMeshComponent::StreamMeshComponent(const resource::Proxy< StreamMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_frame(0)
{
}

void StreamMeshComponent::destroy()
{
	m_mesh.clear();
	m_instance = nullptr;
	MeshComponent::destroy();
}

Aabb3 StreamMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void StreamMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (m_frame >= m_mesh->getFrameCount())
		return;

	if (!m_instance || m_mesh.changed())
	{
		m_instance = m_mesh->createInstance();
		if (!m_instance)
			return;
		m_mesh.consume();
	}

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
		worldRenderPass,
		transform,
		m_instance,
		m_frame,
		distance,
		m_parameterCallback
	);
}

uint32_t StreamMeshComponent::getFrameCount() const
{
	return m_mesh->getFrameCount();
}

void StreamMeshComponent::setFrame(uint32_t frame)
{
	m_frame = frame;
}

uint32_t StreamMeshComponent::getFrame() const
{
	return m_frame;
}

}
