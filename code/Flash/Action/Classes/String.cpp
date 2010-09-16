#include "Flash/Action/Classes/String.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.String", String, ActionObject)

String::String()
:	ActionObject(L"String")
{
}

String::String(wchar_t ch)
:	ActionObject(L"String")
{
	m_str.resize(1, ch);
}

String::String(const std::wstring& str)
:	ActionObject(L"String")
,	m_str(str)
{
}

std::wstring String::toString() const
{
	return m_str;
}

	}
}
