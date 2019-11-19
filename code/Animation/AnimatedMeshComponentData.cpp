#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimatedMeshComponentData", 0, AnimatedMeshComponentData, world::IEntityComponentData)

AnimatedMeshComponentData::AnimatedMeshComponentData()
:	m_normalizePose(false)
,	m_normalizeTransform(false)
,	m_screenSpaceCulling(true)
{
}

Ref< AnimatedMeshComponent > AnimatedMeshComponentData::createComponent(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const
{
	resource::Proxy< mesh::SkinnedMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return nullptr;

	resource::Proxy< Skeleton > skeleton;
	if (!resourceManager->bind(m_skeleton, skeleton))
		return nullptr;

	Ref< IPoseController > poseController;
	if (m_poseController)
		poseController = m_poseController->createInstance(
			resourceManager,
			physicsManager,
			skeleton,
			Transform::identity()
		);

	AlignedVector< int32_t > jointRemap(skeleton->getJointCount());
	const auto& jointMap = mesh->getJointMap();
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		const Joint* joint = skeleton->getJoint(i);
		auto it = jointMap.find(joint->getName());
		if (it == jointMap.end())
		{
			jointRemap[i] = -1;
			continue;
		}
		jointRemap[i] = it->second;
	}

	AlignedVector< AnimatedMeshComponent::Binding > bindings;
	for (size_t i = 0; i < m_bindings.size(); ++i)
	{
		Ref< world::Entity > entity = entityBuilder->create(m_bindings[i].entityData);
		if (entity)
		{
			AnimatedMeshComponent::Binding binding;
			binding.jointHandle = render::getParameterHandle(m_bindings[i].jointName);
			binding.entity = entity;
			bindings.push_back(binding);
		}
	}

	return new AnimatedMeshComponent(
		Transform::identity(),
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

void AnimatedMeshComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::SkinnedMesh >(L"mesh", m_mesh);
	s >> resource::Member< Skeleton >(L"skeleton", m_skeleton);
	s >> MemberRef< const IPoseControllerData >(L"poseController", m_poseController);
	s >> Member< bool >(L"normalizePose", m_normalizePose);
	s >> Member< bool >(L"normalizeTransform", m_normalizeTransform);
	s >> Member< bool >(L"screenSpaceCulling", m_screenSpaceCulling);
	s >> MemberAlignedVector< Binding, MemberComposite< Binding > >(L"bindings", m_bindings);
}

void AnimatedMeshComponentData::Binding::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"jointName", jointName);
	s >> MemberRef< const world::EntityData >(L"entityData", entityData);
}

	}
}
