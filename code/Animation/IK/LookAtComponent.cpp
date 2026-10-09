/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/LookAtComponent.h"

#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Core/Math/Const.h"
#include "Core/Math/Quaternion.h"
#include "World/Entity.h"

#include <cmath>

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.LookAtComponent", LookAtComponent, world::IEntityComponent)

LookAtComponent::LookAtComponent(const Settings& settings)
	: m_settings(settings)
{
}

void LookAtComponent::destroy()
{
}

void LookAtComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void LookAtComponent::setTransform(const Transform& transform)
{
}

Aabb3 LookAtComponent::getBoundingBox() const
{
	return Aabb3();
}

void LookAtComponent::update(const world::UpdateParams& update)
{
	if (m_settings.joint == 0)
		return;

	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (!skeletonComponent)
		return;

	const auto& skeleton = skeletonComponent->getSkeleton();
	if (!skeleton)
		return;

	auto animatedMeshComponent = m_owner->getComponent< AnimatedMeshComponent >();
	if (animatedMeshComponent && animatedMeshComponent->getLastDistance() >= m_settings.cullDistance)
	{
		m_weight = 0.0f;
		return;
	}

	skeletonComponent->synchronize();

	if (skeletonComponent->getRevision() == m_revision)
		return;

	uint32_t index;
	if (!skeleton->findJoint(m_settings.joint, index))
		return;

	const int32_t parent = skeleton->getJoint(index)->getParent();
	if (parent < 0)
		return;

	const AlignedVector< Transform >& jointTransforms = skeletonComponent->getJointTransforms();
	const AlignedVector< Transform >& pose = skeletonComponent->getPoseTransforms();
	if (index >= pose.size() || index >= jointTransforms.size())
		return;

	const Transform ownerTransform = m_owner->getTransform();

	const Vector4 posePosition = pose[index].translation().xyz1();
	const Vector4 targetPosition = ownerTransform.inverse() * m_target.xyz1();

	const Vector4 d = (targetPosition - posePosition).normalized();
	float angle = std::atan2(d.x(), d.z());

	const float limit = deg2rad(30.0f);
	if (angle > limit)
		angle = limit;
	else if (angle < -limit)
		angle = -limit;

	skeletonComponent->setPoseTransform(
		m_settings.joint,
		Transform(
			pose[index].translation(),
			Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f), angle) * jointTransforms[index].rotation()
		),
		true
	);

	m_revision = skeletonComponent->getRevision();
}

}
