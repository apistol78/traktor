#include "Flash/Action/Classes/String.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.String", String, ActionObject)

String::String()
:	ActionObject("String")
{
}

String::String(char ch)
:	ActionObject("String")
{
	m_str.resize(1, ch);
}

String::String(const std::string& str)
:	ActionObject("String")
,	m_str(str)
{
}

ActionValue String::toString() const
{
	return ActionValue(m_str);
}

	}
}
