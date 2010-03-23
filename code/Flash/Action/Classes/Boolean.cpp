#include "Flash/Action/Classes/Boolean.h"
#include "Flash/Action/Avm1/Classes/AsBoolean.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Boolean", Boolean, ActionObject)

Boolean::Boolean(bool value)
:	ActionObject(AsBoolean::getInstance())
,	m_value(value)
{
}

	}
}
