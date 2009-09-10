#include "Ddc/DfnNamespace.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_CLASS(L"ddc.DfnNamespace", DfnNamespace, Object)

DfnNamespace::DfnNamespace(const std::wstring& name, DfnNode* statements)
:	m_name(name)
,	m_statements(statements)
{
}

}
