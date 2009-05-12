#include "Animation/Pose.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.Pose", Pose, Serializable)

void Pose::setBoneOffset(uint32_t boneIndex, const Vector4& boneOffset)
{
	Bone& bone = getEditBone(boneIndex);
	bone.offset = boneOffset;
}

Vector4 Pose::getBoneOffset(uint32_t boneIndex) const
{
	const Bone* bone = getBone(boneIndex);
	return bone ? bone->offset : Vector4::zero();
}

void Pose::setBoneOrientation(uint32_t boneIndex, const Quaternion& boneOrientation)
{
	Bone& bone = getEditBone(boneIndex);
	bone.orientation = boneOrientation;
}

Quaternion Pose::getBoneOrientation(uint32_t boneIndex) const
{
	const Bone* bone = getBone(boneIndex);
	return bone ? bone->orientation : Quaternion::identity();
}

void Pose::getIndexMask(BitSet& outIndices) const
{
	for (AlignedVector< Bone >::const_iterator i = m_bones.begin(); i != m_bones.end(); ++i)
		outIndices.set(i->index);
}

const Pose::Bone* Pose::getBone(uint32_t boneIndex) const
{
	uint32_t s = 0;
	uint32_t e = uint32_t(m_bones.size());

	while (s < e)
	{
		uint32_t m = s + (e - s) / 2;
		if (boneIndex == m_bones[m].index)
			return &m_bones[m];
		else if (boneIndex < m_bones[m].index)
			e = m;
		else if (boneIndex > m_bones[m].index)
			s = m + 1;
	}

	return 0;
}

Pose::Bone& Pose::getEditBone(uint32_t boneIndex)
{
	uint32_t s = 0;
	uint32_t e = uint32_t(m_bones.size());

	while (s < e)
	{
		uint32_t m = s + (e - s) / 2;
		if (boneIndex == m_bones[m].index)
			return m_bones[m];
		else if (boneIndex < m_bones[m].index)
			e = m;
		else if (boneIndex > m_bones[m].index)
			s = m + 1;
	}

	return *m_bones.insert(m_bones.begin() + s, Bone(boneIndex));
}

bool Pose::serialize(Serializer& s)
{
	return s >> MemberAlignedVector< Bone, MemberComposite< Bone > >(L"bones", m_bones);
}

bool Pose::Bone::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"index", index);
	s >> Member< Vector4 >(L"offset", offset);
	s >> Member< Quaternion >(L"orientation", orientation);
	return true;
}

	}
}
