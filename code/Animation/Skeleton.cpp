#include <algorithm>
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.Skeleton", 0, Skeleton, ISerializable)

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

void Skeleton::findChildren(uint32_t index, std::vector< uint32_t >& outChildren) const
{
	for (uint32_t i = 0; i < uint32_t(m_bones.size()); ++i)
	{
		if (m_bones[i]->getParent() == index)
		{
			outChildren.push_back(i);
			findChildren(i, outChildren);
		}
	}
}

bool Skeleton::serialize(ISerializer& s)
{
	return s >> MemberRefArray< Bone >(L"bones", m_bones);
}

	}
}
