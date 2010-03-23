#include "Flash/Action/Classes/String.h"
#include "Flash/Action/Avm1/Classes/AsString.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.String", String, ActionObject)

String::String()
:	ActionObject(AsString::getInstance())
{
}

String::String(wchar_t ch)
:	ActionObject(AsString::getInstance())
{
	m_str.resize(1, ch);
}

String::String(const std::wstring& str)
:	ActionObject(AsString::getInstance())
,	m_str(str)
{
}

std::wstring String::toString() const
{
	return m_str;
}

	}
}
