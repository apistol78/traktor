/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimatedMeshEntityData", 3, AnimatedMeshEntityData, world::EntityData)

AnimatedMeshEntityData::AnimatedMeshEntityData()
:	m_normalizePose(false)
,	m_normalizeTransform(false)
,	m_screenSpaceCulling(true)
{
}

Ref< AnimatedMeshEntity > AnimatedMeshEntityData::createEntity(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const
{
	resource::Proxy< mesh::SkinnedMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return 0;

	resource::Proxy< Skeleton > skeleton;
	if (!resourceManager->bind(m_skeleton, skeleton))
		return 0;

	Ref< IPoseController > poseController;
	if (m_poseController)
		poseController = m_poseController->createInstance(
			resourceManager,
			physicsManager,
			skeleton,
			getTransform()
		);

	std::vector< int32_t > jointRemap(skeleton->getJointCount());
	const std::map< std::wstring, int32_t >& jointMap = mesh->getJointMap();
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		const Joint* joint = skeleton->getJoint(i);
		std::map< std::wstring, int32_t >::const_iterator j = jointMap.find(joint->getName());
		if (j == jointMap.end())
		{
			jointRemap[i] = -1;
			continue;
		}
		jointRemap[i] = j->second;
	}

	std::vector< AnimatedMeshEntity::Binding > bindings;
	for (size_t i = 0; i < m_bindings.size(); ++i)
	{
		Ref< world::Entity > entity = entityBuilder->create(m_bindings[i].entityData);
		if (entity)
		{
			AnimatedMeshEntity::Binding binding;
			binding.jointHandle = render::getParameterHandle(m_bindings[i].jointName);
			binding.entity = entity;
			bindings.push_back(binding);
		}
	}

	return new AnimatedMeshEntity(
		getTransform(),
		mesh,
		skeleton,
		poseController,
		jointRemap,
		bindings,
		m_normalizePose,
		m_normalizeTransform,
		m_screenSpaceCulling
	);
}

void AnimatedMeshEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	
	s >> resource::Member< mesh::SkinnedMesh >(L"mesh", m_mesh);
	s >> resource::Member< Skeleton >(L"skeleton", m_skeleton);
	s >> MemberRef< IPoseControllerData >(L"poseController", m_poseController);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"normalizePose", m_normalizePose);
		s >> Member< bool >(L"normalizeTransform", m_normalizeTransform);
	}

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"screenSpaceCulling", m_screenSpaceCulling);

	if (s.getVersion() >= 3)
		s >> MemberStlVector< Binding, MemberComposite< Binding > >(L"bindings", m_bindings);
}

void AnimatedMeshEntityData::Binding::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"jointName", jointName);
	s >> MemberRef< world::EntityData >(L"entityData", entityData);
}

	}
}
