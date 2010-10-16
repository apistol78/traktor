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

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue As_flash_geom_Point::construct(ActionContext* context, const ActionValueArray& args)
{
	Ref< Point > pt = new Point();

	if (args.size() >= 2)
	{
		pt->x = args[0].getNumberSafe();
		pt->y = args[1].getNumberSafe();
	}

	return ActionValue(pt);
}

void As_flash_geom_Point::Point_add(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	Ref< Point > pr = ca.args[0].getObject< Point >();
	pt->x += pr->x;
	pt->y += pr->y;
}

void As_flash_geom_Point::Point_clone(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	ca.ret = ActionValue(new Point(pt->x, pt->y));
}

void As_flash_geom_Point::Point_distance(CallArgs& ca)
{
	Ref< Point > pt1 = ca.args[0].getObject< Point >();
	Ref< Point > pt2 = ca.args[1].getObject< Point >();
	avm_number_t dx = pt2->x - pt1->x;
	avm_number_t dy = pt2->y - pt1->y;
	ca.ret = ActionValue(sqrtf(dx * dx + dy * dy));
}

void As_flash_geom_Point::Point_equals(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	Ref< Point > pr = ca.args[0].getObject< Point >();
	ca.ret = ActionValue(bool(pt->x == pr->x && pt->y == pr->y));
}

void As_flash_geom_Point::Point_interpolate(CallArgs& ca)
{
	Ref< Point > pt1 = ca.args[0].getObject< Point >();
	Ref< Point > pt2 = ca.args[1].getObject< Point >();
	avm_number_t f = ca.args[2].getNumberSafe();
	ca.ret = ActionValue(new Point(
		pt1->x * (1.0f - f) + pt2->x * f,
		pt1->y * (1.0f - f) + pt2->y * f
	));
}

void As_flash_geom_Point::Point_normalize(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	avm_number_t scale = ca.args[0].getNumberSafe();
	avm_number_t ln = sqrtf(pt->x * pt->x + pt->y * pt->y);
	pt->x *= scale / ln;
	pt->y *= scale / ln;
}

void As_flash_geom_Point::Point_offset(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	pt->x += ca.args[0].getNumberSafe();
	pt->y += ca.args[1].getNumberSafe();
}

void As_flash_geom_Point::Point_polar(CallArgs& ca)
{
	avm_number_t length = ca.args[0].getNumberSafe();
	avm_number_t angle = ca.args[1].getNumberSafe();
	ca.ret = ActionValue(new Point(
		cosf(angle) * length,
		sinf(angle) * length
	));
}

void As_flash_geom_Point::Point_subtract(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	Ref< Point > pr = ca.args[0].getObject< Point >();
	pt->x -= pr->x;
	pt->y -= pr->y;
}

void As_flash_geom_Point::Point_toString(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	ca.ret = pt->toString();
}

void As_flash_geom_Point::Point_get_length(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	ca.ret = ActionValue(sqrtf(pt->x * pt->x + pt->y * pt->y));
}

void As_flash_geom_Point::Point_set_length(CallArgs& ca)
{
}

void As_flash_geom_Point::Point_get_x(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	ca.ret = ActionValue(pt->x);
}

void As_flash_geom_Point::Point_set_x(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	pt->x = ca.args[0].getNumberSafe();
}

void As_flash_geom_Point::Point_get_y(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	ca.ret = ActionValue(pt->y);
}

void As_flash_geom_Point::Point_set_y(CallArgs& ca)
{
	Point* pt = checked_type_cast< Point*, false >(ca.self);
	pt->y = ca.args[0].getNumberSafe();
}

	}
}
