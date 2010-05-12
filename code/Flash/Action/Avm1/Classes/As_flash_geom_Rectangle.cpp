#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Rectangle.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Classes/Point.h"
#include "Flash/Action/Classes/Rectangle.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Rectangle", As_flash_geom_Rectangle, ActionClass)

Ref< As_flash_geom_Rectangle > As_flash_geom_Rectangle::getInstance()
{
	static Ref< As_flash_geom_Rectangle > instance = 0;
	if (!instance)
	{
		instance = new As_flash_geom_Rectangle();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

As_flash_geom_Rectangle::As_flash_geom_Rectangle()
:	ActionClass(L"Rectangle")
{
}

void As_flash_geom_Rectangle::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"clone", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_clone)));
	prototype->setMember(L"contains", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_contains)));
	prototype->setMember(L"containsPoint", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_containsPoint)));
	prototype->setMember(L"containsRectangle", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_containsRectangle)));
	prototype->setMember(L"equals", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_equals)));
	prototype->setMember(L"inflate", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_inflate)));
	prototype->setMember(L"inflatePoint", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_inflatePoint)));
	prototype->setMember(L"intersection", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_intersection)));
	prototype->setMember(L"intersects", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_intersects)));
	prototype->setMember(L"isEmpty", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_isEmpty)));
	prototype->setMember(L"offset", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_offset)));
	prototype->setMember(L"offsetPoint", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_offsetPoint)));
	prototype->setMember(L"setEmpty", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_setEmpty)));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_toString)));
	prototype->setMember(L"union", ActionValue(createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_union)));

	prototype->addProperty(L"bottom", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_bottom), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_bottom));
	prototype->addProperty(L"bottomRight", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_bottomRight), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_bottomRight));
	prototype->addProperty(L"height", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_height), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_height));
	prototype->addProperty(L"left", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_left), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_left));
	prototype->addProperty(L"right", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_right), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_right));
	prototype->addProperty(L"size", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_size), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_size));
	prototype->addProperty(L"top", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_top), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_top));
	prototype->addProperty(L"topLeft", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_topLeft), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_topLeft));
	prototype->addProperty(L"width", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_width), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_width));
	prototype->addProperty(L"x", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_x), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_x));
	prototype->addProperty(L"y", createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_get_y), createNativeFunction(this, &As_flash_geom_Rectangle::Rectangle_set_y));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
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
	ca.ret = ActionValue(rc->toString());
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
