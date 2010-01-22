#include "Flash/Action/Avm1/ActionPoint.h"
#include "Flash/Action/Avm1/Classes/AsPoint.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionPoint", ActionPoint, ActionObject)

ActionPoint::ActionPoint()
:	ActionObject(AsPoint::getInstance())
,	x(0.0)
,	y(0.0)
{
}

ActionPoint::ActionPoint(avm_number_t x_, avm_number_t y_)
:	ActionObject(AsPoint::getInstance())
,	x(x_)
,	y(y_)
{
}

	}
}
