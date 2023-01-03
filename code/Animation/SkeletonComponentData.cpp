/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/SkeletonComponent.h"
#include "Animation/SkeletonComponentData.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.SkeletonComponentData", 0, SkeletonComponentData, world::IEntityComponentData)

SkeletonComponentData::SkeletonComponentData(
	const resource::Id< Skeleton >& skeleton,
	const IPoseControllerData* poseController
)
:	m_skeleton(skeleton)
,	m_poseController(poseController)
{
}

Ref< SkeletonComponent > SkeletonComponentData::createComponent(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const
{
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

	AlignedVector< SkeletonComponent::Binding > bindings;
	for (size_t i = 0; i < m_bindings.size(); ++i)
	{
		Ref< world::Entity > entity = entityBuilder->create(m_bindings[i].entityData);
		if (entity)
		{
			SkeletonComponent::Binding binding;
			binding.jointHandle = render::getParameterHandle(m_bindings[i].jointName);
			binding.entity = entity;
			bindings.push_back(binding);
		}
	}

	return new SkeletonComponent(
		Transform::identity(),
		skeleton,
		poseController,
		bindings
	);
}

void SkeletonComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void SkeletonComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< Skeleton >(L"skeleton", m_skeleton);
	s >> MemberRef< const IPoseControllerData >(L"poseController", m_poseController);
	s >> MemberAlignedVector< Binding, MemberComposite< Binding > >(L"bindings", m_bindings);
}

void SkeletonComponentData::Binding::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"jointName", jointName);
	s >> MemberRef< const world::EntityData >(L"entityData", entityData);
}

}
