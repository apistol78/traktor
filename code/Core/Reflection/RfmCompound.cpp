/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

void RfmCompound::removeMember(ReflectionMember* member)
{
	m_members.remove(member);
}

uint32_t RfmCompound::getMemberCount() const
{
	return uint32_t(m_members.size());
}

ReflectionMember* RfmCompound::getMember(uint32_t index) const
{
	if (index < m_members.size())
		return m_members[index];
	else
		return 0;
}

const RefArray< ReflectionMember >& RfmCompound::getMembers() const
{
	return m_members;
}

ReflectionMember* RfmCompound::findMember(const ReflectionMemberPredicate& predicate) const
{
	for (RefArray< ReflectionMember >::const_iterator i = m_members.begin(); i != m_members.end(); ++i)
	{
		if (predicate(*i))
			return *i;
	}
	return 0;
}

void RfmCompound::findMembers(const ReflectionMemberPredicate& predicate, RefArray< ReflectionMember >& outMembers) const
{
	for (RefArray< ReflectionMember >::const_iterator i = m_members.begin(); i != m_members.end(); ++i)
	{
		if (predicate(*i))
			outMembers.push_back(*i);
		if (const RfmCompound* compoundMember = dynamic_type_cast< const RfmCompound* >(*i))
			compoundMember->findMembers(predicate, outMembers);
	}
}

bool RfmCompound::replace(const ReflectionMember* source)
{
	if (const RfmCompound* sourceCompound = dynamic_type_cast< const RfmCompound* >(source))
	{
		m_members = sourceCompound->m_members;
		return true;
	}
	else
		return false;
}

}
