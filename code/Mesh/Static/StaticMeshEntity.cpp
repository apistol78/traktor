/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{
		
static render::handle_t s_techniqueVelocityWrite = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshEntity", StaticMeshEntity, MeshEntity)

StaticMeshEntity::StaticMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< StaticMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
{
	s_techniqueVelocityWrite = render::getParameterHandle(L"World_VelocityWrite");
}

Aabb3 StaticMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool StaticMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void StaticMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	Transform worldTransform = m_transform.get(worldRenderView.getInterval());
	Transform lastWorldTransform = m_transform.get(worldRenderView.getInterval() - 1.0f);

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (worldTransform == lastWorldTransform)
			return;
	}

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		lastWorldTransform,
		worldTransform,
		distance,
		getParameterCallback()
	);
}

	}
}
