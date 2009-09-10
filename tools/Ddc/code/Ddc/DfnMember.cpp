#include "Ddc/DfnMember.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_CLASS(L"ddc.DfnMember", DfnMember, Object)

DfnMember::DfnMember(DfnNode* type, const std::wstring& name)
:	m_type(type)
,	m_name(name)
{
}

}
