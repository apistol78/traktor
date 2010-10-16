#include "Flash/Action/Classes/Boolean.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Boolean", Boolean, ActionObject)

Boolean::Boolean(bool value)
:	ActionObject("Boolean")
,	m_value(value)
{
}

	}
}
