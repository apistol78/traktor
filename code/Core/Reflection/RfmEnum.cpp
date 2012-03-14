#include "Core/Reflection/RfmEnum.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmEnum", RfmEnum, ReflectionMember)

RfmEnum::RfmEnum(const wchar_t* name, const std::wstring& value)
:	ReflectionMember(name)
,	m_value(value)
{
}

}
