#include <cmath>
#include "Flash/Action/Avm1/ActionRectangle.h"
#include "Flash/Action/Avm1/Classes/AsRectangle.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionRectangle", ActionRectangle, ActionObject)

ActionRectangle::ActionRectangle()
:	ActionObject(AsRectangle::getInstance())
,	left(0.0)
,	top(0.0)
,	right(0.0)
,	bottom(0.0)
{
}

ActionRectangle::ActionRectangle(avm_number_t left_, avm_number_t top_, avm_number_t right_, avm_number_t bottom_)
:	ActionObject(AsRectangle::getInstance())
,	left(left_)
,	top(top_)
,	right(right_)
,	bottom(bottom_)
{
}

bool ActionRectangle::contains(avm_number_t x, avm_number_t y) const
{
	return (x >= left && y >= top && x <= right && y <= bottom);
}

void ActionRectangle::inflate(avm_number_t dx, avm_number_t dy)
{
	left -= dx;
	top -= dy;
	right += dx;
	bottom -= dy;
}

bool ActionRectangle::isEmpty() const
{
	avm_number_t dx = right - left;
	avm_number_t dy = bottom - top;
	return std::abs(dx * dy) < FUZZY_EPSILON;
}

void ActionRectangle::offset(avm_number_t dx, avm_number_t dy)
{
	left += dx;
	top += dy;
	right += dx;
	bottom += dy;
}

	}
}
