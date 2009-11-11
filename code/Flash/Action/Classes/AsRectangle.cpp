#include "Flash/Action/Classes/AsRectangle.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionRectangle.h"
#include "Flash/Action/ActionPoint.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsRectangle", AsRectangle, ActionClass)

Ref< AsRectangle > AsRectangle::getInstance()
{
	static AsRectangle* instance = 0;
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
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"clone", createNativeFunctionValue(this, &AsRectangle::Rectangle_clone));
	prototype->setMember(L"contains", createNativeFunctionValue(this, &AsRectangle::Rectangle_contains));
	prototype->setMember(L"containsPoint", createNativeFunctionValue(this, &AsRectangle::Rectangle_containsPoint));
	prototype->setMember(L"containsRectangle", createNativeFunctionValue(this, &AsRectangle::Rectangle_containsRectangle));
	prototype->setMember(L"equals", createNativeFunctionValue(this, &AsRectangle::Rectangle_equals));
	prototype->setMember(L"inflate", createNativeFunctionValue(this, &AsRectangle::Rectangle_inflate));
	prototype->setMember(L"inflatePoint", createNativeFunctionValue(this, &AsRectangle::Rectangle_inflatePoint));
	prototype->setMember(L"intersection", createNativeFunctionValue(this, &AsRectangle::Rectangle_intersection));
	prototype->setMember(L"intersects", createNativeFunctionValue(this, &AsRectangle::Rectangle_intersects));
	prototype->setMember(L"isEmpty", createNativeFunctionValue(this, &AsRectangle::Rectangle_isEmpty));
	prototype->setMember(L"offset", createNativeFunctionValue(this, &AsRectangle::Rectangle_offset));
	prototype->setMember(L"offsetPoint", createNativeFunctionValue(this, &AsRectangle::Rectangle_offsetPoint));
	prototype->setMember(L"setEmpty", createNativeFunctionValue(this, &AsRectangle::Rectangle_setEmpty));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsRectangle::Rectangle_toString));
	prototype->setMember(L"union", createNativeFunctionValue(this, &AsRectangle::Rectangle_union));

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

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsRectangle::construct(ActionContext* context, const args_t& args)
{
	Ref< ActionRectangle > rc = gc_new< ActionRectangle >();
	
	if (args.size() >= 4)
	{
		rc->left = args[0].getNumberSafe();
		rc->top = args[1].getNumberSafe();
		rc->right = rc->left + args[2].getNumberSafe();
		rc->bottom = rc->top + args[3].getNumberSafe();
	}

	return ActionValue::fromObject(rc);
}

void AsRectangle::Rectangle_clone(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue::fromObject(gc_new< ActionRectangle >(
		rc->left,
		rc->top,
		rc->right,
		rc->bottom
	));
}

void AsRectangle::Rectangle_contains(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->contains(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	));
}

void AsRectangle::Rectangle_containsPoint(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	Ref< ActionPoint > pt = ca.args[0].getObject< ActionPoint >();
	ca.ret = ActionValue(rc->contains(
		pt->x,
		pt->y
	));
}

void AsRectangle::Rectangle_containsRectangle(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	Ref< ActionRectangle > rt = ca.args[0].getObject< ActionRectangle >();
	ca.ret = ActionValue(
		rc->contains(rt->left, rt->top) &&
		rc->contains(rt->left, rt->bottom) &&
		rc->contains(rt->right, rt->top) &&
		rc->contains(rt->right, rt->bottom)
	);
}

void AsRectangle::Rectangle_equals(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
}

void AsRectangle::Rectangle_inflate(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->inflate(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void AsRectangle::Rectangle_inflatePoint(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	Ref< ActionPoint > pt = ca.args[0].getObject< ActionPoint >();
	rc->inflate(pt->x, pt->y);
}

void AsRectangle::Rectangle_intersection(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
}

void AsRectangle::Rectangle_intersects(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
}

void AsRectangle::Rectangle_isEmpty(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->isEmpty());
}

void AsRectangle::Rectangle_offset(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->offset(
		ca.args[0].getNumberSafe(),
		ca.args[1].getNumberSafe()
	);
}

void AsRectangle::Rectangle_offsetPoint(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	Ref< ActionPoint > pt = ca.args[0].getObject< ActionPoint >();
	rc->offset(pt->x, pt->y);
}

void AsRectangle::Rectangle_setEmpty(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->left =
	rc->top =
	rc->right =
	rc->bottom = 0.0;
}

void AsRectangle::Rectangle_toString(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	StringOutputStream ss; ss << rc->left << L", " << rc->top << L" - " << (rc->right - rc->left) << L", " << (rc->bottom - rc->top);
	ca.ret = ActionValue(ss.str());
}

void AsRectangle::Rectangle_union(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
}

void AsRectangle::Rectangle_get_bottom(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->bottom);
}

void AsRectangle::Rectangle_set_bottom(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->bottom = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_bottomRight(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue::fromObject(gc_new< ActionPoint >(rc->right, rc->bottom));
}

void AsRectangle::Rectangle_set_bottomRight(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	Ref< ActionPoint > pt = ca.args[0].getObject< ActionPoint >();
	rc->right = pt->x;
	rc->bottom = pt->y;
}

void AsRectangle::Rectangle_get_height(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->bottom - rc->top);
}

void AsRectangle::Rectangle_set_height(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->bottom = rc->top + ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_left(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->left);
}

void AsRectangle::Rectangle_set_left(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->left = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_right(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->right);
}

void AsRectangle::Rectangle_set_right(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->right = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_size(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue::fromObject(gc_new< ActionPoint >(rc->right - rc->left, rc->bottom - rc->top));
}

void AsRectangle::Rectangle_set_size(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	Ref< ActionPoint > pt = ca.args[0].getObject< ActionPoint >();
	rc->right = rc->left + pt->x;
	rc->bottom = rc->top + pt->y;
}

void AsRectangle::Rectangle_get_top(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->top);
}

void AsRectangle::Rectangle_set_top(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->top = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_topLeft(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue::fromObject(gc_new< ActionPoint >(rc->left, rc->top));
}

void AsRectangle::Rectangle_set_topLeft(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	Ref< ActionPoint > pt = ca.args[0].getObject< ActionPoint >();
	rc->left = pt->x;
	rc->top = pt->y;
}

void AsRectangle::Rectangle_get_width(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->right - rc->left);
}

void AsRectangle::Rectangle_set_width(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->right = rc->left + ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_x(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->left);
}

void AsRectangle::Rectangle_set_x(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->right = ca.args[0].getNumberSafe() + (rc->right - rc->left);
	rc->left = ca.args[0].getNumberSafe();
}

void AsRectangle::Rectangle_get_y(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	ca.ret = ActionValue(rc->top);
}

void AsRectangle::Rectangle_set_y(CallArgs& ca)
{
	Ref< ActionRectangle > rc = checked_type_cast< ActionRectangle* >(ca.self);
	rc->bottom = ca.args[0].getNumberSafe() + (rc->bottom - rc->top);
	rc->top = ca.args[0].getNumberSafe();
}

	}
}
