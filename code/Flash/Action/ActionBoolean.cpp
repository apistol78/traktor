#include "Flash/Action/ActionBoolean.h"
#include "Flash/Action/Classes/AsBoolean.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionBoolean", ActionBoolean, ActionObject)

ActionBoolean::ActionBoolean(bool value)
:	ActionObject(AsBoolean::getInstance())
,	m_value(value)
{
}

	}
}
