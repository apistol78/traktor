#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfpMemberType", RfpMemberType, ReflectionMemberPredicate)

RfpMemberType::RfpMemberType(const TypeInfo& memberType)
:	m_memberType(memberType)
{
}

bool RfpMemberType::operator () (const ReflectionMember* member) const
{
	T_ASSERT (member);
	const TypeInfo& memberType = type_of(member);
	return is_type_of(m_memberType, memberType);
}

}
