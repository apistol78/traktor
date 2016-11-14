#include "Flash/ColorTransform.h"
#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Transform.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Transform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Transform", As_flash_geom_Transform, ActionClass)

As_flash_geom_Transform::As_flash_geom_Transform(ActionContext* context)
:	ActionClass(context, "flash.geom.Transform")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->addProperty("colorTransform", createNativeFunction(context, this, &As_flash_geom_Transform::Transform_get_colorTransform), createNativeFunction(context, this, &As_flash_geom_Transform::Transform_set_colorTransform));
	//prototype->addProperty("concatenatedColorTransform", createNativeFunction(context, this, &Transform_get_concatenatedColorTransform), 0);
	//prototype->addProperty("concatenatedMatrix", createNativeFunction(context, this, &Transform_get_concatenatedMatrix), 0);
	//prototype->addProperty("matrix", createNativeFunction(context, this, &Transform_get_matrix), createNativeFunction(context, this, &Transform_set_matrix));
	//prototype->addProperty("pixelBounds", createNativeFunction(context, this, &Transform_get_pixelBounds), createNativeFunction(context, this, &Transform_set_pixelBounds));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_Transform::initialize(ActionObject* self)
{
}

void As_flash_geom_Transform::construct(ActionObject* self, const ActionValueArray& args)
{
	Ref< Transform > tf;
	if (args.size() >= 1 && args[0].isObject())
	{
		Ref< FlashCharacterInstance > instance = args[0].getObject()->getRelay< FlashCharacterInstance >();
		if (instance)
			tf = new Transform(instance);
	}
	self->setRelay(tf);
}

ActionValue As_flash_geom_Transform::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

Ref< ColorTransform > As_flash_geom_Transform::Transform_get_colorTransform(Transform* self) const
{
	return self->getColorTransform();
}

void As_flash_geom_Transform::Transform_set_colorTransform(Transform* self, const ColorTransform* colorTransform) const
{
	self->setColorTransform(colorTransform);
}

	}
}
