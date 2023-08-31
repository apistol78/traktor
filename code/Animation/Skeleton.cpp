/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Animation/Skeleton.h"
#include "Animation/Joint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Skeleton", 0, Skeleton, ISerializable)

int32_t Skeleton::addJoint(Joint* joint)
{
	const int32_t jointIndex = int32_t(m_joints.size());
	m_joints.push_back(joint);
	m_jointMap.insert(
		render::getParameterHandle(joint->getName()),
		jointIndex
	);
	return jointIndex;
}

void Skeleton::removeJoint(Joint* joint)
{
	auto itj = std::find(m_joints.begin(), m_joints.end(), joint);
	m_joints.erase(itj);

	auto itm = m_jointMap.find(render::getParameterHandle(joint->getName()));
	m_jointMap.erase(itm);
}

bool Skeleton::findJoint(render::handle_t name, uint32_t& outIndex) const
{
	const auto it = m_jointMap.find(name);
	if (it != m_jointMap.end())
	{
		outIndex = it->second;
		return true;
	}
	else
		return false;
}

void Skeleton::findChildren(uint32_t index, const std::function< void(uint32_t) >& fn) const
{
	for (uint32_t i = 0; i < (uint32_t)m_joints.size(); ++i)
	{
		if (m_joints[i]->getParent() == index)
			fn(i);
	}
}

void Skeleton::findAllChildren(uint32_t index, const std::function< void(uint32_t) >& fn) const
{
	for (uint32_t i = 0; i < (uint32_t)m_joints.size(); ++i)
	{
		if (m_joints[i]->getParent() == index)
		{
			fn(i);
			findAllChildren(i, fn);
		}
	}
}

void Skeleton::serialize(ISerializer& s)
{
	s >> MemberRefArray< Joint >(L"joints", m_joints);
	if (s.getDirection() == ISerializer::Direction::Read)
	{
		for (uint32_t i = 0; i < (uint32_t)m_joints.size(); ++i)
		{
			const Joint* joint = m_joints[i];
			if (joint)
				m_jointMap.insert(render::getParameterHandle(joint->getName()), i);
		}
	}
}

}
