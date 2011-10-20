#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Classes/Point.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Point", Point, ActionObjectRelay)

Point::Point()
:	ActionObjectRelay("flash.geom.Point")
,	x(0.0)
,	y(0.0)
{
}

Point::Point(avm_number_t x_, avm_number_t y_)
:	ActionObjectRelay("flash.geom.Point")
,	x(x_)
,	y(y_)
{
}

	}
}
