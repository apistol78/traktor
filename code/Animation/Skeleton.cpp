#include <algorithm>
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.animation.Skeleton", Skeleton, Serializable)

uint32_t Skeleton::addBone(Bone* bone)
{
	uint32_t boneIndex = uint32_t(m_bones.size());
	m_bones.push_back(bone);
	return boneIndex;
}

void Skeleton::removeBone(Bone* bone)
{
	RefArray< Bone >::iterator i = std::find(m_bones.begin(), m_bones.end(), bone);
	m_bones.erase(i);
}

bool Skeleton::findBone(const std::wstring& name, uint32_t& outIndex) const
{
	for (uint32_t i = 0; i < uint32_t(m_bones.size()); ++i)
	{
		if (m_bones[i]->getName() == name)
		{
			outIndex = i;
			return true;
		}
	}
	return false;
}

bool Skeleton::serialize(Serializer& s)
{
	return s >> MemberRefArray< Bone >(L"bones", m_bones);
}

	}
}
