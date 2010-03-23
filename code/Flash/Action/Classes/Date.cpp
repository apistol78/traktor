#include "Flash/Action/Classes/Date.h"
#include "Flash/Action/Avm1/Classes/AsDate.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Date", Date, ActionObject)

Date::Date()
:	ActionObject(AsDate::getInstance())
{
}

	}
}
