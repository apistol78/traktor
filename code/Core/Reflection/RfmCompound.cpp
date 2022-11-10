/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/ReflectionMemberPredicate.h"
#include "Core/Reflection/RfmCompound.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmCompound", RfmCompound, ReflectionMember)

RfmCompound::RfmCompound(const wchar_t* name)
:	ReflectionMember(name)
{
}

void RfmCompound::addMember(ReflectionMember* member)
{
	m_members.push_back(member);
}

bool RfmCompound::removeMember(ReflectionMember* member)
{
	return m_members.remove(member);
}

uint32_t RfmCompound::getMemberCount() const
{
	return (uint32_t)m_members.size();
}

ReflectionMember* RfmCompound::getMember(uint32_t index) const
{
	if (index < m_members.size())
		return m_members[index];
	else
		return nullptr;
}

const RefArray< ReflectionMember >& RfmCompound::getMembers() const
{
	return m_members;
}

ReflectionMember* RfmCompound::findMember(const ReflectionMemberPredicate& predicate) const
{
	for (auto member : m_members)
	{
		if (predicate(member))
			return member;
	}
	return nullptr;
}

void RfmCompound::findMembers(const ReflectionMemberPredicate& predicate, RefArray< ReflectionMember >& outMembers) const
{
	outMembers.reserve(outMembers.capacity() + m_members.size());
	for (auto member : m_members)
	{
		if (predicate(member))
			outMembers.push_back(member);
		if (const RfmCompound* compoundMember = dynamic_type_cast< const RfmCompound* >(member))
			compoundMember->findMembers(predicate, outMembers);
	}
}

bool RfmCompound::replace(const ReflectionMember* source)
{
	if (auto sourceCompound = dynamic_type_cast< const RfmCompound* >(source))
	{
		m_members = sourceCompound->m_members;
		return true;
	}
	else
		return false;
}

}
