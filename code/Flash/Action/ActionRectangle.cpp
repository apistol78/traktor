#include <cmath>
#include "Flash/Action/ActionRectangle.h"
#include "Flash/Action/Classes/AsRectangle.h"
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

ActionRectangle::ActionRectangle(double left_, double top_, double right_, double bottom_)
:	ActionObject(AsRectangle::getInstance())
,	left(left_)
,	top(top_)
,	right(right_)
,	bottom(bottom_)
{
}

bool ActionRectangle::contains(double x, double y) const
{
	return (x >= left && y >= top && x <= right && y <= bottom);
}

void ActionRectangle::inflate(double dx, double dy)
{
	left -= dx;
	top -= dy;
	right += dx;
	bottom -= dy;
}

bool ActionRectangle::isEmpty() const
{
	double dx = right - left;
	double dy = bottom - top;
	return std::abs(dx * dy) < FUZZY_EPSILON;
}

void ActionRectangle::offset(double dx, double dy)
{
	left += dx;
	top += dy;
	right += dx;
	bottom += dy;
}

	}
}
