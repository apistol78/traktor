/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/IK/IKComponent.h"
#include "Core/Containers/StaticVector.h"
#include "World/Entity.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.IKComponent", IKComponent, world::IEntityComponent)

void IKComponent::destroy()
{
}

void IKComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void IKComponent::setTransform(const Transform& transform)
{
}

Aabb3 IKComponent::getBoundingBox() const
{
	return Aabb3();
}

void IKComponent::update(const world::UpdateParams& update)
{
	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (!skeletonComponent)
		return;

	auto skeleton = skeletonComponent->getSkeleton();
	if (!skeleton)
		return;

	const uint32_t jointCount = skeleton->getJointCount();
	if (jointCount > 128)
		return;

	const auto& jointTransforms = skeletonComponent->getJointTransforms();
	const auto& poseTransforms = skeletonComponent->getPoseTransforms();
	if (poseTransforms.size() < jointCount)
		return;

	StaticVector< Vector4, 128 > nodes(jointCount);
	StaticVector< Scalar, 128 > lengths(jointCount, 0.0_simd);

	// Calculate skeleton bone lengths.
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		// Node position.
		nodes[i] = poseTransforms[i].translation().xyz1();

		// Node to parent bone length.
		const Joint* joint = skeleton->getJoint(i);
		if (joint->getParent() >= 0)
		{
			const Vector4 s = jointTransforms[joint->getParent()].translation();
			const Vector4 e = jointTransforms[i].translation();
			lengths[i] = (e - s).length();
		}
	}

	// Get target indices.
	StaticVector< std::pair< uint32_t, Vector4 >, 8 > targets;
	for (const auto& it : m_targets)
	{
		uint32_t index;
		if (skeleton->findJoint(it.first, index))
		{
			targets.push_back({
				index,
				it.second
			});
		}
	}

	// Solve IK by iteratively solving each constraint individually.
	for (uint32_t i = 0; i < 8/*m_solverIterations*/; ++i)
	{
		// Constraint; keep target position.
		for (const auto& it : targets)
			nodes[it.first] = it.second;

		// Constraint; keep length.
		for (uint32_t j = 0; j < jointCount; ++j)
		{
			const Joint* joint = skeleton->getJoint(j);
			if (joint->getParent() < 0)
				continue;

			Vector4& s = nodes[joint->getParent()];
			Vector4& e = nodes[j];

			Vector4 d = e - s;
			const Scalar ln = d.length();
			const Scalar err = lengths[j] - ln;
			if (abs(err) > FUZZY_EPSILON)
			{
				d /= ln;
				e += err * d * 0.5_simd;
				s -= err * d * 0.5_simd;
			}
		}
	}

	// Update pose transforms from node system.
	AlignedVector< Transform > solvedPoseTransforms;
	solvedPoseTransforms.resize(jointCount);
	
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		solvedPoseTransforms[i] = Transform(
			nodes[i].xyz0(),
			poseTransforms[i].rotation()
		);
	}

	for (uint32_t i = 0; i < jointCount; ++i)
	{
		const Joint* joint = skeleton->getJoint(i);
		if (joint->getParent() >= 0)
		{
			const Vector4& sik = nodes[joint->getParent()];
			const Vector4& eik = nodes[i];
			const Vector4 axisZik = (eik - sik).normalized();

			const Vector4 s = poseTransforms[joint->getParent()].translation();
			const Vector4 e = poseTransforms[i].translation();
			const Vector4 axisZik0 = (e - s).normalized();

			const Quaternion Qr(axisZik0, axisZik);

			solvedPoseTransforms[joint->getParent()] = Transform(
				sik,
				Qr.normalized() * poseTransforms[joint->getParent()].rotation()
			);
		}
	}

	// Replace pose transforms.
	skeletonComponent->setPoseTransforms(solvedPoseTransforms);
}

void IKComponent::setTarget(render::handle_t jointName, const Vector4& position)
{
	m_targets[jointName] = position;
}

}
