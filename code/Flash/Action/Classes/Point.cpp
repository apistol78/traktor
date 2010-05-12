#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/Classes/Point.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Point.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Point", Point, ActionObject)

Point::Point()
:	ActionObject(As_flash_geom_Point::getInstance())
,	x(0.0)
,	y(0.0)
{
}

Point::Point(avm_number_t x_, avm_number_t y_)
:	ActionObject(As_flash_geom_Point::getInstance())
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
