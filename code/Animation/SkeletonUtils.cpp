/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Animation/Joint.h"
#include "Animation/Pose.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"

namespace traktor
{
	namespace animation
	{

void calculateJointLocalTransforms(
	const Skeleton* skeleton,
	AlignedVector< Transform >& outJointLocalTransforms
)
{
	T_ASSERT(skeleton);
	outJointLocalTransforms.resize(skeleton->getJointCount());
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
		outJointLocalTransforms[i] = skeleton->getJoint(i)->getTransform();
}

void calculateJointTransforms(
	const Skeleton* skeleton,
	AlignedVector< Transform >& outJointTransforms
)
{
	T_ASSERT(skeleton);

	AlignedVector< Transform > localJointTransforms;
	calculateJointLocalTransforms(skeleton, localJointTransforms);

	outJointTransforms.resize(skeleton->getJointCount());
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		outJointTransforms[i] = localJointTransforms[i];
		for (int32_t parentIndex = skeleton->getJoint(i)->getParent(); parentIndex >= 0; parentIndex = skeleton->getJoint(parentIndex)->getParent())
			outJointTransforms[i] = localJointTransforms[parentIndex] * outJointTransforms[i];
	}
}

void calculatePoseLocalTransforms(
	const Skeleton* skeleton,
	const Pose* pose,
	AlignedVector< Transform >& outJointLocalTransforms
)
{
	T_ASSERT(skeleton);
	T_ASSERT(pose);

	outJointLocalTransforms.resize(skeleton->getJointCount());
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
		outJointLocalTransforms[i] = pose->getJointTransform(i);
}

void calculatePoseTransforms(
	const Skeleton* skeleton,
	const Pose* pose,
	AlignedVector< Transform >& outJointTransforms
)
{
	T_ASSERT(skeleton);
	T_ASSERT(pose);

	AlignedVector< Transform > localPoseTransforms;
	calculatePoseLocalTransforms(skeleton, pose, localPoseTransforms);

	outJointTransforms.resize(skeleton->getJointCount());
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		outJointTransforms[i] = localPoseTransforms[i];
		for (int32_t parentIndex = skeleton->getJoint(i)->getParent(); parentIndex >= 0; parentIndex = skeleton->getJoint(parentIndex)->getParent())
			outJointTransforms[i] = localPoseTransforms[parentIndex] * outJointTransforms[i];
	}
}

Aabb3 calculateBoundingBox(const Skeleton* skeleton)
{
	Aabb3 boundingBox;

	AlignedVector< Transform > jointTransforms;
	calculateJointTransforms(skeleton, jointTransforms);

	for (uint32_t i = 0; i < uint32_t(jointTransforms.size()); ++i)
	{
		const Joint* joint = skeleton->getJoint(i);
		float radius = joint->getRadius();

		Aabb3 jointLocalAabb(Vector4(-radius, -radius, -radius), Vector4(radius, radius, radius));
		Aabb3 jointAabb = jointLocalAabb.transform(jointTransforms[i]);

		boundingBox.contain(jointAabb);
	}

	return boundingBox;
}

Aabb3 calculateBoundingBox(const Skeleton* skeleton, const Pose* pose)
{
	Aabb3 boundingBox;

	AlignedVector< Transform > poseTransforms;
	calculatePoseTransforms(skeleton, pose, poseTransforms);

	for (uint32_t i = 0; i < uint32_t(poseTransforms.size()); ++i)
	{
		const Joint* joint = skeleton->getJoint(i);
		float radius = joint->getRadius();

		Aabb3 jointLocalAabb(Vector4(-radius, -radius, -radius), Vector4(radius, radius, radius));
		Aabb3 jointAabb = jointLocalAabb.transform(poseTransforms[i]);

		boundingBox.contain(jointAabb);
	}

	return boundingBox;
}

void blendPoses(
	const Pose* pose1,
	const Pose* pose2,
	const Scalar& blend,
	Pose* outPose
)
{
	T_ASSERT(pose1);
	T_ASSERT(pose2);
	T_ASSERT(outPose);

	// Build mask of all used joint indices.
	BitSet indices;
	pose1->getIndexMask(indices);
	pose2->getIndexMask(indices);

	int minRange, maxRange;
	indices.range(minRange, maxRange);

	outPose->reset();
	outPose->reserve(maxRange - minRange);
	for (int i = minRange; i < maxRange; ++i)
	{
		if (!indices(i))
			continue;

		Transform t1 = pose1->getJointTransform(i);
		Transform t2 = pose2->getJointTransform(i);
		outPose->setJointTransform(i, lerp(t1, t2, blend));
	}
}

	}
}
