/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/MeshCulling.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMeshComponent", SkinnedMeshComponent, MeshComponent)

SkinnedMeshComponent::SkinnedMeshComponent(const resource::Proxy< SkinnedMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_count(0)
{
	const std::map< std::wstring, int >& jointMap = m_mesh->getJointMap();
	m_jointTransforms[0].resize(jointMap.size() * 2, Vector4::origo());
	m_jointTransforms[1].resize(jointMap.size() * 2, Vector4::origo());

	s_techniqueVelocityWrite = render::getParameterHandle(L"World_VelocityWrite");
}

void SkinnedMeshComponent::destroy()
{
	m_mesh.clear();
	m_jointTransforms[0].clear();
	m_jointTransforms[1].clear();
	MeshComponent::destroy();
}

Aabb3 SkinnedMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void SkinnedMeshComponent::render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass)
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

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		lastWorldTransform,
		worldTransform,
		m_jointTransforms[m_count],
		distance,
		m_parameterCallback
	);
}

void SkinnedMeshComponent::setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms_)
{
	AlignedVector< Vector4 >& jointTransforms = m_jointTransforms[m_count];

	const std::map< std::wstring, int >& jointMap = m_mesh->getJointMap();
	jointTransforms.resize(jointMap.size() * 2, Vector4::origo());

	uint32_t size = uint32_t(jointTransforms_.size());
	for (uint32_t i = 0, j = 0; i < size; i += 2, ++j)
	{
		if (j < jointTransforms_.size())
		{
			Transform joint(jointTransforms_[i]);
			jointTransforms[i + 0] = joint.rotation().e;
			jointTransforms[i + 1] = joint.translation().xyz1();
		}
		else
		{
			jointTransforms[i + 0] = Vector4::origo();
			jointTransforms[i + 1] = Vector4::origo();
		}
	}

	Atomic::exchange(m_count, 1 - m_count);
}

	}
}
