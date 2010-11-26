#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "Ddc/DfnMember.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ddc.DfnMember", 0, DfnMember, DfnNode)

DfnMember::DfnMember()
{
}

DfnMember::DfnMember(DfnNode* type, const std::wstring& name)
:	m_type(type)
,	m_name(name)
{
}

bool DfnMember::serialize(ISerializer& s)
{
	s >> MemberRef< DfnNode >(L"type", m_type);
	s >> Member< std::wstring >(L"name", m_name);
	return DfnNode::serialize(s);
}

}
