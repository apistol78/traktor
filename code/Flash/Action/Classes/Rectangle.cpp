#include <cmath>
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Const.h"
#include "Flash/Action/Classes/Rectangle.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Rectangle.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Rectangle", Rectangle, ActionObject)

Rectangle::Rectangle()
:	ActionObject(As_flash_geom_Rectangle::getInstance())
,	left(0.0)
,	top(0.0)
,	width(0.0)
,	height(0.0)
{
}

Rectangle::Rectangle(avm_number_t left_, avm_number_t top_, avm_number_t width_, avm_number_t height_)
:	ActionObject(As_flash_geom_Rectangle::getInstance())
,	left(left_)
,	top(top_)
,	width(width_)
,	height(height_)
{
}

bool Rectangle::contains(avm_number_t x, avm_number_t y) const
{
	return (x >= left && y >= top && x <= left + width && y <= top + height);
}

void Rectangle::inflate(avm_number_t dx, avm_number_t dy)
{
	left -= dx;
	top -= dy;
	width += dx * 2;
	height -= dy * 2;
}

bool Rectangle::isEmpty() const
{
	return std::abs(width * height) < FUZZY_EPSILON;
}

void Rectangle::offset(avm_number_t dx, avm_number_t dy)
{
	left += dx;
	top += dy;
}

std::wstring Rectangle::toString() const
{
	StringOutputStream ss;
	ss << L"(x=" << left << L", y=" << top << L", w=" << width << L", h=" << height << L")";
	return ss.str();
}

	}
}
