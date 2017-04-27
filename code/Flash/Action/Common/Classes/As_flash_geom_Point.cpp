/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Point.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Point.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Point", As_flash_geom_Point, ActionClass)

As_flash_geom_Point::As_flash_geom_Point(ActionContext* context)
:	ActionClass(context, "flash.geom.Point")
{
	setMember("distance", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_distance)));
	setMember("interpolate", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_interpolate)));
	setMember("polar", ActionValue(createNativeFunction(context, this, &As_flash_geom_Point::Point_polar)));

	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("add", ActionValue(createPolymorphicFunction(context, 0, createNativeFunction(context, &Point::add_1), createNativeFunction(context, &Point::add_2))));
	prototype->setMember("clone", ActionValue(createNativeFunction(context, &Point::clone)));
	prototype->setMember("equals", ActionValue(createNativeFunction(context, &Point::equals)));
	prototype->setMember("normalize", ActionValue(createNativeFunction(context, &Point::normalize)));
	prototype->setMember("offset", ActionValue(createNativeFunction(context, &Point::offset)));
	prototype->setMember("subtract", ActionValue(createNativeFunction(context, &Point::subtract)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, &Point::toString)));

	prototype->addProperty("length", createNativeFunction(context, &Point::getLength), createNativeFunction(context, &Point::setLength));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_Point::initialize(ActionObject* self)
{
}

void As_flash_geom_Point::construct(ActionObject* self, const ActionValueArray& args)
{
	float v[2] = { 0.0f };

	if (args.size() > 0)
		v[0] = args[0].getFloat();
	if (args.size() > 1)
		v[1] = args[1].getFloat();

	self->setRelay(new Point(v[0], v[1]));
}

ActionValue As_flash_geom_Point::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void As_flash_geom_Point::Point_distance(CallArgs& ca)
{
	ActionObject* apt1 = ca.args[0].getObject();
	ActionObject* apt2 = ca.args[1].getObject();

	Point* pt1 = apt1->getRelay< Point >();
	Point* pt2 = apt2->getRelay< Point >();

	if (pt1 && pt2)
		ca.ret = ActionValue(Point::distance(*pt1, *pt2));
}

void As_flash_geom_Point::Point_interpolate(CallArgs& ca)
{
	ActionObject* apt1 = ca.args[0].getObject();
	ActionObject* apt2 = ca.args[1].getObject();

	Point* pt1 = apt1->getRelay< Point >();
	Point* pt2 = apt2->getRelay< Point >();

	float f = ca.args[2].getFloat();

	if (pt1 && pt2)
		ca.ret = ActionValue(new ActionObject(ca.context, Point::interpolate(*pt1, *pt2, f)));
}

void As_flash_geom_Point::Point_polar(CallArgs& ca)
{
	float length = ca.args[0].getFloat();
	float angle = ca.args[1].getFloat();
	ca.ret = ActionValue(new ActionObject(ca.context, Point::polar(length, angle)));
}

	}
}
