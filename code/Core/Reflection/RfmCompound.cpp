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
	return m_members.size();
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

}
