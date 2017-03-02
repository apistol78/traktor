#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Point.h"
#include "Flash/Action/Common/Rectangle.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Rectangle.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Rectangle", As_flash_geom_Rectangle, ActionClass)

As_flash_geom_Rectangle::As_flash_geom_Rectangle(ActionContext* context)
:	ActionClass(context, "flash.geom.Rectangle")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("clone", ActionValue(createNativeFunction(context, &Rectangle::clone)));
	prototype->setMember("contains", ActionValue(createNativeFunction(context, &Rectangle::contains)));
	prototype->setMember("containsPoint", ActionValue(createNativeFunction(context, &Rectangle::containsPoint)));
	prototype->setMember("containsRectangle", ActionValue(createNativeFunction(context, &Rectangle::containsRectangle)));
	prototype->setMember("equals", ActionValue(createNativeFunction(context, &Rectangle::equals)));
	prototype->setMember("inflate", ActionValue(createNativeFunction(context, &Rectangle::inflate)));
	prototype->setMember("inflatePoint", ActionValue(createNativeFunction(context, &Rectangle::inflatePoint)));
	prototype->setMember("intersection", ActionValue(createNativeFunction(context, &Rectangle::intersection)));
	prototype->setMember("intersects", ActionValue(createNativeFunction(context, &Rectangle::intersects)));
	prototype->setMember("isEmpty", ActionValue(createNativeFunction(context, &Rectangle::isEmpty)));
	prototype->setMember("offset", ActionValue(createNativeFunction(context, &Rectangle::offset)));
	prototype->setMember("offsetPoint", ActionValue(createNativeFunction(context, &Rectangle::offsetPoint)));
	prototype->setMember("setEmpty", ActionValue(createNativeFunction(context, &Rectangle::setEmpty)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, &Rectangle::toString)));
	prototype->setMember("union", ActionValue(createNativeFunction(context, &Rectangle::union_)));

	prototype->addProperty("bottom", createNativeFunction(context, &Rectangle::getBottom), createNativeFunction(context, &Rectangle::setBottom));
	prototype->addProperty("bottomRight", createNativeFunction(context, &Rectangle::getBottomRight), createNativeFunction(context, &Rectangle::setBottomRight));
	prototype->addProperty("left", createNativeFunction(context, &Rectangle::getLeft), createNativeFunction(context, &Rectangle::setLeft));
	prototype->addProperty("right", createNativeFunction(context, &Rectangle::getRight), createNativeFunction(context, &Rectangle::setRight));
	prototype->addProperty("size", createNativeFunction(context, &Rectangle::getSize), createNativeFunction(context, &Rectangle::setSize));
	prototype->addProperty("top", createNativeFunction(context, &Rectangle::getTop), createNativeFunction(context, &Rectangle::setTop));
	prototype->addProperty("topLeft", createNativeFunction(context, &Rectangle::getTopLeft), createNativeFunction(context, &Rectangle::setTopLeft));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_Rectangle::initialize(ActionObject* self)
{
}

void As_flash_geom_Rectangle::construct(ActionObject* self, const ActionValueArray& args)
{
	float v[4] = { 0.0f };

	if (args.size() > 0)
		v[0] = args[0].getFloat();
	if (args.size() > 1)
		v[1] = args[1].getFloat();
	if (args.size() > 2)
		v[2] = args[2].getFloat();
	if (args.size() > 3)
		v[3] = args[30].getFloat();

	self->setRelay(new Rectangle(v));
}

ActionValue As_flash_geom_Rectangle::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
