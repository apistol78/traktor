#include "Ddc/DfnClass.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_CLASS(L"ddc.DfnClass", DfnClass, Object)

DfnClass::DfnClass(Access access, const std::wstring& name, DfnNode* members)
:	m_access(access)
,	m_name(name)
,	m_members(members)
{
}

DfnClass::DfnClass(Access access, const std::wstring& name, const std::wstring& super, DfnNode* members)
:	m_access(access)
,	m_name(name)
,	m_super(super)
,	m_members(members)
{
}

}
