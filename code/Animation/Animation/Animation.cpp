/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"

#include "Animation/SkeletonUtils.h"
#include "Core/Math/Const.h"
#include "Core/Math/Hermite.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Animation", 3, Animation, ISerializable)

uint32_t Animation::addKeyPose(const KeyPose& pose)
{
	uint32_t poseIndex = 0;
	for (auto i = m_poses.begin(); i != m_poses.end(); ++i)
	{
		if (pose.at < i->at)
		{
			m_poses.insert(i, pose);
			return poseIndex;
		}
		++poseIndex;
	}
	m_poses.push_back(pose);
	return poseIndex;
}

void Animation::removeKeyPose(uint32_t poseIndex)
{
	T_ASSERT(poseIndex < uint32_t(m_poses.size()));
	m_poses.erase(m_poses.begin() + size_t(poseIndex));
}

bool Animation::empty() const
{
	return m_poses.empty();
}

uint32_t Animation::getKeyPoseCount() const
{
	return uint32_t(m_poses.size());
}

Animation::KeyPose& Animation::getKeyPose(uint32_t poseIndex)
{
	T_ASSERT(poseIndex < uint32_t(m_poses.size()));
	return m_poses[poseIndex];
}

const Animation::KeyPose& Animation::getKeyPose(uint32_t poseIndex) const
{
	T_ASSERT(poseIndex < uint32_t(m_poses.size()));
	return m_poses[poseIndex];
}

const Animation::KeyPose& Animation::getLastKeyPose() const
{
	T_ASSERT(!empty());
	return m_poses.back();
}

bool Animation::getPose(float at, Pose& outPose) const
{
	const size_t nposes = m_poses.size();
	if (nposes > 2)
	{
		int32_t index = -1;
		int32_t index0 = 0;
		int32_t index1 = (int32_t)(nposes - 1);

		while (index0 < index1)
		{
			index = (index0 + index1) / 2;

			const float Tkey0 = m_poses[index].at;
			const float Tkey1 = (index + 1) < nposes ? m_poses[index + 1].at : std::numeric_limits< float >::max();

			if (at < Tkey0)
				index1 = index;
			else if (at > Tkey1)
				index0 = index + 1;
			else
				break;
		}

		const Scalar k((at - m_poses[index].at) / (m_poses[index + 1].at - m_poses[index].at));
		blendPoses(
			&m_poses[index].pose,
			&m_poses[index + 1].pose,
			clamp(k, 0.0_simd, 1.0_simd),
			&outPose);

		return true;
	}
	else if (nposes > 1)
	{
		if (at <= m_poses[0].at)
		{
			outPose = m_poses[0].pose;
			return true;
		}
		else if (at >= m_poses[1].at)
		{
			outPose = m_poses[1].pose;
			return true;
		}
		else
		{
			blendPoses(
				&m_poses[0].pose,
				&m_poses[1].pose,
				Scalar((at - m_poses[0].at) / (m_poses[1].at - m_poses[0].at)),
				&outPose);
			return true;
		}
	}
	else if (nposes > 0)
	{
		outPose = m_poses[0].pose;
		return true;
	}
	else
		return false;
}

Vector4 Animation::getRootMotion(float at) const
{
	if (!haveRootMotion())
		return Vector4::zero();

	const size_t nposes = m_poses.size();
	if (nposes == 1 || at <= m_poses[0].at)
		return m_rootMotion[0];
	if (at >= m_poses[nposes - 1].at)
		return m_rootMotion[nposes - 1];

	// Key poses are sorted by time; find the pair bracketing the time.
	size_t index1 = 1;
	while (index1 < nposes - 1 && m_poses[index1].at < at)
		++index1;
	const size_t index0 = index1 - 1;

	const float span = m_poses[index1].at - m_poses[index0].at;
	const float k = span > FUZZY_EPSILON ? (at - m_poses[index0].at) / span : 0.0f;
	return lerp(m_rootMotion[index0], m_rootMotion[index1], Scalar(clamp(k, 0.0f, 1.0f)));
}

void Animation::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< Animation >() >= 3);

	s >> MemberAlignedVector< KeyPose, MemberComposite< KeyPose > >(L"poses", m_poses);
	s >> MemberAlignedVector< Vector4 >(L"rootMotion", m_rootMotion);
}

void Animation::KeyPose::serialize(ISerializer& s)
{
	s >> Member< float >(L"at", at, AttributeRange(0.0f));
	s >> MemberComposite< Pose >(L"pose", pose);
}

}
