/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Math/Hermite.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

struct KeyPoseAccessor
{
	static inline float time(const Animation::KeyPose* keys, size_t nkeys, const Animation::KeyPose& key)
	{
		return key.at;
	}

	static inline Pose value(const Animation::KeyPose& key)
	{
		return key.pose;
	}

	static inline Pose combine(
		float t,
		const Pose& v0, float w0,
		const Pose& v1, float w1,
		const Pose& v2, float w2,
		const Pose& v3, float w3
	)
	{
		Pose pose;

		// Build mask of all used joint indices.
		BitSet indices;
		v0.getIndexMask(indices);
		v1.getIndexMask(indices);
		v2.getIndexMask(indices);
		v3.getIndexMask(indices);

		int minRange, maxRange;
		indices.range(minRange, maxRange);

		const Scalar sw0(w0);
		const Scalar sw1(w1);
		const Scalar sw2(w2);
		const Scalar sw3(w3);

		for (int i = minRange; i < maxRange; ++i)
		{
			if (!indices(i))
				continue;

			const auto& t0 = v0.getJointTransform(i);
			const auto& t1 = v1.getJointTransform(i);
			const auto& t2 = v2.getJointTransform(i);
			const auto& t3 = v3.getJointTransform(i);

			pose.setJointTransform(
				i,
				Transform(
					t0.translation() * sw0 + t1.translation() * sw1 + t2.translation() * sw2 + t3.translation() * sw3,
					(t0.rotation() * sw0 + t1.rotation() * sw1 + t2.rotation() * sw2 + t3.rotation() * sw3).normalized()
				)
			);
		}

		return pose;
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Animation", 0, Animation, ISerializable)

uint32_t Animation::addKeyPose(const KeyPose& pose)
{
	uint32_t poseIndex = 0;
	for (AlignedVector< KeyPose >::iterator i = m_poses.begin(); i != m_poses.end(); ++i)
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

bool Animation::getPose(float at, bool linear, int32_t& indexHint, Pose& outPose) const
{
	const size_t nposes = m_poses.size();
	if (nposes > 2)
	{
		if (!linear)
			outPose = Hermite< KeyPose, Pose, KeyPoseAccessor, ClampTime >(&m_poses[0], nposes).evaluate(at);
		else
		{
			int32_t index = -1;

			if (indexHint >= 0 && indexHint < nposes - 1)
			{
				const float Tkey0 = m_poses[indexHint].at;
				const float Tkey1 = m_poses[indexHint + 1].at;
				if (at >= Tkey0 && at <= Tkey1)
					index = indexHint;
				else if (at > Tkey1 && indexHint < nposes - 2)
					index = indexHint + 1;
			}

			if (index < 0)
			{
				int32_t index0 = 0;
				int32_t index1 = int32_t(nposes - 2);

				while (index0 < index1)
				{
					index = (index0 + index1) / 2;

					float Tkey0 = m_poses[index].at;
					float Tkey1 = m_poses[index + 1].at;

					if (at < Tkey0)
						index1 = index - 1;
					else if (at > Tkey1)
						index0 = index + 1;
					else
						break;
				}
			}

			const Scalar k((at - m_poses[index].at) / (m_poses[index + 1].at - m_poses[index].at));

			blendPoses(
				&m_poses[index].pose,
				&m_poses[index + 1].pose,
				clamp(k, 0.0_simd, 1.0_simd),
				&outPose
			);

			indexHint = index;
		}
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
				&outPose
			);
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

void Animation::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< KeyPose, MemberComposite< KeyPose > >(L"poses", m_poses);
}

void Animation::KeyPose::serialize(ISerializer& s)
{
	s >> Member< float >(L"at", at, AttributeRange(0.0f));
	s >> MemberComposite< Pose >(L"pose", pose);
}

	}
}
