#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Rectangle.h"
#include "Flash/Action/Classes/Point.h"
#include "Flash/Action/Classes/Rectangle.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Rectangle", As_flash_geom_Rectangle, ActionClass)

As_flash_geom_Rectangle::As_flash_geom_Rectangle()
:	ActionClass("flash.geom.Rectangle")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("clone", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_clone)));
	prototype->setMember("contains", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_contains)));
	prototype->setMember("containsPoint", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_containsPoint)));
	prototype->setMember("containsRectangle", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_containsRectangle)));
	prototype->setMember("equals", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_equals)));
	prototype->setMember("inflate", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_inflate)));
	prototype->setMember("inflatePoint", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_inflatePoint)));
	prototype->setMember("intersection", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_intersection)));
	prototype->setMember("intersects", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_intersects)));
	prototype->setMember("isEmpty", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_isEmpty)));
	prototype->setMember("offset", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_offset)));
	prototype->setMember("offsetPoint", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_offsetPoint)));
	prototype->setMember("setEmpty", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_setEmpty)));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_toString)));
	prototype->setMember("union", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_union)));

	prototype->addProperty("bottom", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_bottom), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_bottom));
	prototype->addProperty("bottomRight", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_bottomRight), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_bottomRight));
	prototype->addProperty("height", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_height), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_height));
	prototype->addProperty("left", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_left), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_left));
	prototype->addProperty("right", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_right), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_right));
	prototype->addProperty("size", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_size), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_size));
	prototype->addProperty("top", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_top), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_top));
	prototype->addProperty("topLeft", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_topLeft), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_topLeft));
	prototype->addProperty("width", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_width), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_width));
	prototype->addProperty("x", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_x), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_x));
	prototype->addProperty("y", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_y), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_y));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue As_flash_geom_Rectangle::construct(ActionContext* context, const ActionValueArray& args)
{
	Ref< Rectangle > rc = new Rectangle();
	
	if (args.size() >= 4)
	{
		rc->left = args[0].getNumberSafe();
		rc->top = args[1].getNumberSafe();
		rc->width = args[2].getNumberSafe();
		rc->height = args[3].getNumberSafe();
	}

	return ActionValue(rc);
}

void As_flash_geom_Rectangle::Rectangle_clone(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Rectangle(
		rc->left,
		rc->top,
		rc->width,
		rc->height
	));
}

void As_flash_geom_Rectangle::Rectangle_contains(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->contains(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	));
}

void As_flash_geom_Rectangle::Rectangle_containsPoint(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	ca.ret = ActionValue(rc->contains(
		pt->x,
		pt->y
	));
}

void As_flash_geom_Rectangle::Rectangle_containsRectangle(CallArgs& ca)
{
}

void As_flash_geom_Rectangle::Rectangle_equals(CallArgs& ca)
{
}

void As_flash_geom_Rectangle::Rectangle_inflate(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->inflate(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void As_flash_geom_Rectangle::Rectangle_inflatePoint(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->inflate(pt->x, pt->y);
}

void As_flash_geom_Rectangle::Rectangle_intersection(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
}

void As_flash_geom_Rectangle::Rectangle_intersects(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
}

void As_flash_geom_Rectangle::Rectangle_isEmpty(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->isEmpty());
}

void As_flash_geom_Rectangle::Rectangle_offset(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->offset(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void As_flash_geom_Rectangle::Rectangle_offsetPoint(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->offset(pt->x, pt->y);
}

void As_flash_geom_Rectangle::Rectangle_setEmpty(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->left =
	rc->top =
	rc->width =
	rc->height = 0.0;
}

void As_flash_geom_Rectangle::Rectangle_toString(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = rc->toString();
}

void As_flash_geom_Rectangle::Rectangle_union(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
}

void As_flash_geom_Rectangle::Rectangle_get_bottom(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->top + rc->height);
}

void As_flash_geom_Rectangle::Rectangle_set_bottom(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->height = ca.args[0].getNumberSafe() - rc->top;
}

void As_flash_geom_Rectangle::Rectangle_get_bottomRight(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Point(rc->left + rc->width, rc->top + rc->height));
}

void As_flash_geom_Rectangle::Rectangle_set_bottomRight(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->width = pt->x - rc->left;
	rc->height = pt->y - rc->top;
}

void As_flash_geom_Rectangle::Rectangle_get_height(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->height);
}

void As_flash_geom_Rectangle::Rectangle_set_height(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->height = ca.args[0].getNumberSafe();
}

void As_flash_geom_Rectangle::Rectangle_get_left(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->left);
}

void As_flash_geom_Rectangle::Rectangle_set_left(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->left = ca.args[0].getNumberSafe();
}

void As_flash_geom_Rectangle::Rectangle_get_right(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->left + rc->width);
}

void As_flash_geom_Rectangle::Rectangle_set_right(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->width = rc->left + ca.args[0].getNumberSafe();
}

void As_flash_geom_Rectangle::Rectangle_get_size(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Point(rc->width, rc->height));
}

void As_flash_geom_Rectangle::Rectangle_set_size(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->width = pt->x;
	rc->height = pt->y;
}

void As_flash_geom_Rectangle::Rectangle_get_top(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->top);
}

void As_flash_geom_Rectangle::Rectangle_set_top(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->top = ca.args[0].getNumberSafe();
}

void As_flash_geom_Rectangle::Rectangle_get_topLeft(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Point(rc->left, rc->top));
}

void As_flash_geom_Rectangle::Rectangle_set_topLeft(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->left = pt->x;
	rc->top = pt->y;
}

void As_flash_geom_Rectangle::Rectangle_get_width(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->width);
}

void As_flash_geom_Rectangle::Rectangle_set_width(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->width = ca.args[0].getNumberSafe();
}

void As_flash_geom_Rectangle::Rectangle_get_x(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->left);
}

void As_flash_geom_Rectangle::Rectangle_set_x(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->left = ca.args[0].getNumberSafe();
}

void As_flash_geom_Rectangle::Rectangle_get_y(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->top);
}

void As_flash_geom_Rectangle::Rectangle_set_y(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->top = ca.args[0].getNumberSafe();
}

	}
}
