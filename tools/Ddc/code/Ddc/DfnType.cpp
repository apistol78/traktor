#include "Ddc/DfnType.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_CLASS(L"ddc.DfnType", DfnType, Object)

DfnType::DfnType(const std::wstring& name, bool isArray)
:	m_name(name)
,	m_isArray(isArray)
{
}

}
