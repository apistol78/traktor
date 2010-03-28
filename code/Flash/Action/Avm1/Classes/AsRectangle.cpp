#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsRectangle.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Classes/Point.h"
#include "Flash/Action/Classes/Rectangle.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsRectangle", AsRectangle, ActionClass)

Ref< AsRectangle > AsRectangle::getInstance()
{
	static Ref< AsRectangle > instance = 0;
	if (!instance)
	{
		instance = new AsRectangle();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsRectangle::AsRectangle()
:	ActionClass(L"Rectangle")
{
}

void AsRectangle::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"clone", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_clone)));
	prototype->setMember(L"contains", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_contains)));
	prototype->setMember(L"containsPoint", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_containsPoint)));
	prototype->setMember(L"containsRectangle", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_containsRectangle)));
	prototype->setMember(L"equals", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_equals)));
	prototype->setMember(L"inflate", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_inflate)));
	prototype->setMember(L"inflatePoint", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_inflatePoint)));
	prototype->setMember(L"intersection", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_intersection)));
	prototype->setMember(L"intersects", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_intersects)));
	prototype->setMember(L"isEmpty", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_isEmpty)));
	prototype->setMember(L"offset", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_offset)));
	prototype->setMember(L"offsetPoint", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_offsetPoint)));
	prototype->setMember(L"setEmpty", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_setEmpty)));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_toString)));
	prototype->setMember(L"union", ActionValue(createNativeFunction(this, &AsRectangle::Rectangle_union)));

	prototype->addProperty(L"bottom", createNativeFunction(this, &AsRectangle::Rectangle_get_bottom), createNativeFunction(this, &AsRectangle::Rectangle_set_bottom));
	prototype->addProperty(L"bottomRight", createNativeFunction(this, &AsRectangle::Rectangle_get_bottomRight), createNativeFunction(this, &AsRectangle::Rectangle_set_bottomRight));
	prototype->addProperty(L"height", createNativeFunction(this, &AsRectangle::Rectangle_get_height), createNativeFunction(this, &AsRectangle::Rectangle_set_height));
	prototype->addProperty(L"left", createNativeFunction(this, &AsRectangle::Rectangle_get_left), createNativeFunction(this, &AsRectangle::Rectangle_set_left));
	prototype->addProperty(L"right", createNativeFunction(this, &AsRectangle::Rectangle_get_right), createNativeFunction(this, &AsRectangle::Rectangle_set_right));
	prototype->addProperty(L"size", createNativeFunction(this, &AsRectangle::Rectangle_get_size), createNativeFunction(this, &AsRectangle::Rectangle_set_size));
	prototype->addProperty(L"top", createNativeFunction(this, &AsRectangle::Rectangle_get_top), createNativeFunction(this, &AsRectangle::Rectangle_set_top));
	prototype->addProperty(L"topLeft", createNativeFunction(this, &AsRectangle::Rectangle_get_topLeft), createNativeFunction(this, &AsRectangle::Rectangle_set_topLeft));
	prototype->addProperty(L"width", createNativeFunction(this, &AsRectangle::Rectangle_get_width), createNativeFunction(this, &AsRectangle::Rectangle_set_width));
	prototype->addProperty(L"x", createNativeFunction(this, &AsRectangle::Rectangle_get_x), createNativeFunction(this, &AsRectangle::Rectangle_set_x));
	prototype->addProperty(L"y", createNativeFunction(this, &AsRectangle::Rectangle_get_y), createNativeFunction(this, &AsRectangle::Rectangle_set_y));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsRectangle::construct(ActionContext* context, const args_t& args)
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

void AsRectangle::Rectangle_clone(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Rectangle(
		rc->left,
		rc->top,
		rc->width,
		rc->height
	));
}

void AsRectangle::Rectangle_contains(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->contains(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	));
}

void AsRectangle::Rectangle_containsPoint(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	ca.ret = ActionValue(rc->contains(
		pt->x,
		pt->y
	));
}

void AsRectangle::Rectangle_containsRectangle(CallArgs& ca)
{
}

void AsRectangle::Rectangle_equals(CallArgs& ca)
{
}

void AsRectangle::Rectangle_inflate(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->inflate(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void AsRectangle::Rectangle_inflatePoint(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->inflate(pt->x, pt->y);
}

void AsRectangle::Rectangle_intersection(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
}

void AsRectangle::Rectangle_intersects(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
}

void AsRectangle::Rectangle_isEmpty(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->isEmpty());
}

void AsRectangle::Rectangle_offset(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->offset(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void AsRectangle::Rectangle_offsetPoint(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->offset(pt->x, pt->y);
}

void AsRectangle::Rectangle_setEmpty(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->left =
	rc->top =
	rc->width =
	rc->height = 0.0;
}

void AsRectangle::Rectangle_toString(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->toString());
}

void AsRectangle::Rectangle_union(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
}

void AsRectangle::Rectangle_get_bottom(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->top + rc->height);
}

void AsRectangle::Rectangle_set_bottom(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->height = ca.args[0].getNumberSafe() - rc->top;
}

void AsRectangle::Rectangle_get_bottomRight(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Point(rc->left + rc->width, rc->top + rc->height));
}

void AsRectangle::Rectangle_set_bottomRight(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->width = pt->x - rc->left;
	rc->height = pt->y - rc->top;
}

void AsRectangle::Rectangle_get_height(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->height);
}

void AsRectangle::Rectangle_set_height(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->height = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_left(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->left);
}

void AsRectangle::Rectangle_set_left(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->left = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_right(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->left + rc->width);
}

void AsRectangle::Rectangle_set_right(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->width = rc->left + ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_size(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Point(rc->width, rc->height));
}

void AsRectangle::Rectangle_set_size(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->width = pt->x;
	rc->height = pt->y;
}

void AsRectangle::Rectangle_get_top(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->top);
}

void AsRectangle::Rectangle_set_top(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->top = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_topLeft(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(new Point(rc->left, rc->top));
}

void AsRectangle::Rectangle_set_topLeft(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	Ref< Point > pt = ca.args[0].getObject< Point >();
	rc->left = pt->x;
	rc->top = pt->y;
}

void AsRectangle::Rectangle_get_width(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->width);
}

void AsRectangle::Rectangle_set_width(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->width = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_x(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->left);
}

void AsRectangle::Rectangle_set_x(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->left = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_y(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	ca.ret = ActionValue(rc->top);
}

void AsRectangle::Rectangle_set_y(CallArgs& ca)
{
	Rectangle* rc = checked_type_cast< Rectangle*, false >(ca.self);
	rc->top = ca.args[0].getNumberSafe();
}

	}
}
