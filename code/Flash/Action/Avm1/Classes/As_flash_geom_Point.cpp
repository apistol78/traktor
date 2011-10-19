#include <cmath>
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Point.h"
#include "Flash/Action/Classes/Point.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Point", As_flash_geom_Point, ActionClass)

As_flash_geom_Point::As_flash_geom_Point()
:	ActionClass("flash.geom.Point")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("add", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_add)));
	prototype->setMember("clone", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_clone)));
	prototype->setMember("distance", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_distance)));
	prototype->setMember("equals", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_equals)));
	prototype->setMember("interpolate", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_interpolate)));
	prototype->setMember("normalize", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_normalize)));
	prototype->setMember("offset", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_offset)));
	prototype->setMember("polar", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_polar)));
	prototype->setMember("subtract", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_subtract)));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &As_flash_geom_Point::Point_toString)));

	prototype->addProperty("length", createNativeFunction(this, &As_flash_geom_Point::Point_get_length), createNativeFunction(this, &As_flash_geom_Point::Point_set_length));
	prototype->addProperty("x", createNativeFunction(this, &As_flash_geom_Point::Point_get_x), createNativeFunction(this, &As_flash_geom_Point::Point_set_x));
	prototype->addProperty("y", createNativeFunction(this, &As_flash_geom_Point::Point_get_y), createNativeFunction(this, &As_flash_geom_Point::Point_set_y));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > As_flash_geom_Point::alloc(ActionContext* context)
{
	return new Point();
}

void As_flash_geom_Point::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	if (args.size() < 2)
		return;

	Point* pt = checked_type_cast< Point* >(self);
	pt->x = args[0].getNumber();
	pt->y = args[1].getNumber();
}

void As_flash_geom_Point::Point_add(Point* self, const Point* arg) const
{
	self->x += arg->x;
	self->y += arg->y;
}

Ref< Point > As_flash_geom_Point::Point_clone(const Point* self) const
{
	return new Point(self->x, self->y);
}

avm_number_t As_flash_geom_Point::Point_distance(const Point* pt1, const Point* pt2) const
{
	avm_number_t dx = pt2->x - pt1->x;
	avm_number_t dy = pt2->y - pt1->y;
	return avm_number_t(sqrtf(dx * dx + dy * dy));
}

bool As_flash_geom_Point::Point_equals(const Point* self, const Point* pt) const
{
	return self->x == pt->x && self->y == pt->y;
}

Ref< Point > As_flash_geom_Point::Point_interpolate(Point* self, const Point* pt1, const Point* pt2, avm_number_t f) const
{
	return new Point(
		pt1->x * (1.0f - f) + pt2->x * f,
		pt1->y * (1.0f - f) + pt2->y * f
	);
}

void As_flash_geom_Point::Point_normalize(Point* self, avm_number_t scale) const
{
	avm_number_t ln = sqrtf(self->x * self->x + self->y * self->y);
	self->x *= scale / ln;
	self->y *= scale / ln;
}

void As_flash_geom_Point::Point_offset(Point* self, avm_number_t dx, avm_number_t dy) const
{
	self->x += dx;
	self->y += dy;
}

Ref< Point > As_flash_geom_Point::Point_polar(Point* self, avm_number_t length, avm_number_t angle) const
{
	return new Point(
		cosf(angle) * length,
		sinf(angle) * length
	);
}

void As_flash_geom_Point::Point_subtract(Point* self, const Point* pr) const
{
	self->x -= pr->x;
	self->y -= pr->y;
}

ActionValue As_flash_geom_Point::Point_toString(const Point* self) const
{
	return self->toString();
}

avm_number_t As_flash_geom_Point::Point_get_length(const Point* self) const
{
	return avm_number_t(sqrtf(self->x * self->x + self->y * self->y));
}

void As_flash_geom_Point::Point_set_length(Point* self, avm_number_t length) const
{
	avm_number_t d = length / avm_number_t(sqrtf(self->x * self->x + self->y * self->y));
	self->x *= d;
	self->y *= d;
}

avm_number_t As_flash_geom_Point::Point_get_x(const Point* self) const
{
	return self->x;
}

void As_flash_geom_Point::Point_set_x(Point* self, avm_number_t x) const
{
	self->x = x;
}

avm_number_t As_flash_geom_Point::Point_get_y(const Point* self) const
{
	return self->y;
}

void As_flash_geom_Point::Point_set_y(Point* self, avm_number_t y) const
{
	self->y = y;
}

	}
}
