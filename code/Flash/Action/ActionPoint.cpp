#include "Flash/Action/ActionPoint.h"
#include "Flash/Action/Classes/AsPoint.h"

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

ActionPoint::ActionPoint(double x_, double y_)
:	ActionObject(AsPoint::getInstance())
,	x(x_)
,	y(y_)
{
}

	}
}
