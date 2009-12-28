#include "Flash/Action/Avm1/ActionBoolean.h"
#include "Flash/Action/Avm1/Classes/AsBoolean.h"

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
