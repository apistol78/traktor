#include "Animation/Animation/Animation.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Math/Hermite.h"
#include "Core/Serialization/Serializer.h"
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
	static inline Scalar time(const Animation::KeyPose& key)
	{
		return Scalar(key.at);
	}

	static inline Pose value(const Animation::KeyPose& key)
	{
		return key.pose;
	}

	static inline Pose combine(
		const Pose& v0, const Scalar& w0,
		const Pose& v1, const Scalar& w1,
		const Pose& v2, const Scalar& w2,
		const Pose& v3, const Scalar& w3
	)
	{
		Pose pose;

		// Build mask of all used bone indices.
		BitSet indices;
		v0.getIndexMask(indices);
		v1.getIndexMask(indices);
		v2.getIndexMask(indices);
		v3.getIndexMask(indices);

		int minRange, maxRange;
		indices.range(minRange, maxRange);

		for (int i = minRange; i < maxRange; ++i)
		{
			if (!indices(i))
				continue;

			pose.setBoneOffset(i, v0.getBoneOffset(i) * w0 + v1.getBoneOffset(i) * w1 + v2.getBoneOffset(i) * w2 + v3.getBoneOffset(i) * w3);
			pose.setBoneOrientation(i, (v0.getBoneOrientation(i) * w0 + v1.getBoneOrientation(i) * w1 + v2.getBoneOrientation(i) * w2 + v3.getBoneOrientation(i) * w3).normalized());
		}

		return pose;
	}
};

		}

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.animation.Animation", Animation, Serializable)

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
	T_ASSERT (poseIndex >= 0 && poseIndex < uint32_t(m_poses.size()));
	m_poses.erase(m_poses.begin() + poseIndex);
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
	T_ASSERT (poseIndex >= 0 && poseIndex < uint32_t(m_poses.size()));
	return m_poses[poseIndex];
}

const Animation::KeyPose& Animation::getKeyPose(uint32_t poseIndex) const
{
	T_ASSERT (poseIndex >= 0 && poseIndex < uint32_t(m_poses.size()));
	return m_poses[poseIndex];
}

const Animation::KeyPose& Animation::getLastKeyPose() const
{
	T_ASSERT (!empty());
	return m_poses.back();
}

bool Animation::getPose(float at, Pose& outPose) const
{
	size_t nposes = m_poses.size();
	if (nposes <= 1)
	{
		if (nposes > 0)
		{
			outPose = m_poses.front().pose;
			return true;
		}
		return true;
	}
	outPose = Hermite< KeyPose, Scalar, Pose, KeyPoseAccessor, WrapTime< Scalar > >::evaluate(&m_poses[0], nposes, Scalar(at));
	return true;
}

bool Animation::serialize(Serializer& s)
{
	return s >> MemberAlignedVector< KeyPose, MemberComposite< KeyPose > >(L"poses", m_poses);
}

bool Animation::KeyPose::serialize(Serializer& s)
{
	s >> Member< float >(L"at", at, 0.0f);
	s >> MemberComposite< Pose >(L"pose", pose);
	return true;
}

	}
}
