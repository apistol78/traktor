#include <algorithm>
#include "Animation/Skeleton.h"
#include "Animation/Joint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.Skeleton", 0, Skeleton, ISerializable)

int32_t Skeleton::addJoint(Joint* joint)
{
	int32_t jointIndex = int32_t(m_joints.size());
	m_joints.push_back(joint);
	return jointIndex;
}

void Skeleton::removeJoint(Joint* joint)
{
	RefArray< Joint >::iterator i = std::find(m_joints.begin(), m_joints.end(), joint);
	m_joints.erase(i);
}

bool Skeleton::findJoint(const std::wstring& name, uint32_t& outIndex) const
{
	for (uint32_t i = 0; i < uint32_t(m_joints.size()); ++i)
	{
		if (m_joints[i]->getName() == name)
		{
			outIndex = i;
			return true;
		}
	}
	return false;
}

void Skeleton::findChildren(uint32_t index, std::vector< uint32_t >& outChildren) const
{
	for (uint32_t i = 0; i < uint32_t(m_joints.size()); ++i)
	{
		if (m_joints[i]->getParent() == index)
		{
			outChildren.push_back(i);
			findChildren(i, outChildren);
		}
	}
}

bool Skeleton::serialize(ISerializer& s)
{
	return s >> MemberRefArray< Joint >(L"joints", m_joints);
}

	}
}
