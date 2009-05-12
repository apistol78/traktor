#include "Flash/Action/ActionNumber.h"
#include "Flash/Action/Classes/AsNumber.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionNumber", ActionNumber, ActionObject)

ActionNumber::ActionNumber(double value)
:	ActionObject(AsNumber::getInstance())
,	m_value(value)
{
}

	}
}
