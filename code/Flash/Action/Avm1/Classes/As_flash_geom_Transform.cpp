#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/ColorTransform.h"
#include "Flash/Action/Classes/Transform.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Transform.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Transform", As_flash_geom_Transform, ActionClass)

As_flash_geom_Transform::As_flash_geom_Transform(ActionContext* context)
:	ActionClass(context, "flash.geom.Transform")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->addProperty("colorTransform", createNativeFunction(context, this, &As_flash_geom_Transform::Transform_get_colorTransform), createNativeFunction(context, this, &As_flash_geom_Transform::Transform_set_colorTransform));
	//prototype->addProperty("concatenatedColorTransform", createNativeFunction(context, this, &Transform_get_concatenatedColorTransform), 0);
	//prototype->addProperty("concatenatedMatrix", createNativeFunction(context, this, &Transform_get_concatenatedMatrix), 0);
	//prototype->addProperty("matrix", createNativeFunction(context, this, &Transform_get_matrix), createNativeFunction(context, this, &Transform_set_matrix));
	//prototype->addProperty("pixelBounds", createNativeFunction(context, this, &Transform_get_pixelBounds), createNativeFunction(context, this, &Transform_set_pixelBounds));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setMember("__coerce__", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_Transform::init(ActionObject* self, const ActionValueArray& args) const
{
	Ref< Transform > tf;
	if (args.size() >= 1)
	{
		Ref< FlashCharacterInstance > instance = args[0].getObject< FlashCharacterInstance >();
		if (instance)
			tf = new Transform(instance);
	}
	self->setRelay(tf);
}

void As_flash_geom_Transform::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
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
