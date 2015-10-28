#include "Mesh/MeshCulling.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshComponent.h"
#include "World/IWorldCulling.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StreamMeshComponent", StreamMeshComponent, MeshComponent)

StreamMeshComponent::StreamMeshComponent(world::Entity* owner, bool screenSpaceCulling, const resource::Proxy< StreamMesh >& mesh)
:	MeshComponent(owner, screenSpaceCulling)
,	m_mesh(mesh)
,	m_frame(0)
{
}

void StreamMeshComponent::destroy()
{
	m_mesh.clear();
	m_instance = 0;
	MeshComponent::destroy();
}

Aabb3 StreamMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void StreamMeshComponent::render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass, const Transform& transform)
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

	}
}
