#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/Classes/Point.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Point", Point, ActionObject)

Point::Point()
:	ActionObject(L"flash.geom.Point")
,	x(0.0)
,	y(0.0)
{
}

Point::Point(avm_number_t x_, avm_number_t y_)
:	ActionObject(L"flash.geom.Point")
,	x(x_)
,	y(y_)
{
}

std::wstring Point::toString() const
{
	StringOutputStream ss;
	ss << L"(x=" << x << L", y=" << y << L")";
	return ss.str();
}

	}
}
