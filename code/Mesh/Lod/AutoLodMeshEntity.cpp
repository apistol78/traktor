/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshEntity.h"
#include "World/IWorldRenderPass.h"
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
	Transform worldTransform = m_transform.get(worldRenderView.getInterval());

	const Vector4& eyePosition = worldRenderView.getEyePosition();
	m_lodDistance = (worldTransform.translation() - eyePosition).length();

	m_mesh->render(
		m_lodDistance,
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform.get0(),
		worldTransform,
		distance,
		getParameterCallback()
	);
}

	}
}
