#include "Flash/Action/ActionString.h"
#include "Flash/Action/Classes/AsString.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionString", ActionString, ActionObject)

ActionString::ActionString()
:	ActionObject(AsString::getInstance())
{
}

ActionString::ActionString(wchar_t ch)
:	ActionObject(AsString::getInstance())
{
	m_str.resize(1, ch);
}

ActionString::ActionString(const std::wstring& str)
:	ActionObject(AsString::getInstance())
,	m_str(str)
{
}

std::wstring ActionString::toString() const
{
	return m_str;
}

	}
}
