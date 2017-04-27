/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/Stream/StreamMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StreamMeshEntity", StreamMeshEntity, MeshEntity)

StreamMeshEntity::StreamMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< StreamMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
,	m_frame(0)
{
}

uint32_t StreamMeshEntity::getFrameCount() const
{
	return m_mesh->getFrameCount();
}

void StreamMeshEntity::setFrame(uint32_t frame)
{
	m_frame = frame;
}

Aabb3 StreamMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool StreamMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void StreamMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	if (m_frame >= m_mesh->getFrameCount())
		return;

	if (!m_instance)
	{
		m_instance = m_mesh->createInstance();
		if (!m_instance)
			return;
	}

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform.get(worldRenderView.getInterval()),
		m_instance,
		m_frame,
		distance,
		getParameterCallback()
	);
}

	}
}
