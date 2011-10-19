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

As_flash_geom_Transform::As_flash_geom_Transform()
:	ActionClass("flash.geom.Transform")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->addProperty("colorTransform", createNativeFunction(this, &As_flash_geom_Transform::Transform_get_colorTransform), createNativeFunction(this, &As_flash_geom_Transform::Transform_set_colorTransform));
	//prototype->addProperty("concatenatedColorTransform", createNativeFunction(this, &Transform_get_concatenatedColorTransform), 0);
	//prototype->addProperty("concatenatedMatrix", createNativeFunction(this, &Transform_get_concatenatedMatrix), 0);
	//prototype->addProperty("matrix", createNativeFunction(this, &Transform_get_matrix), createNativeFunction(this, &Transform_set_matrix));
	//prototype->addProperty("pixelBounds", createNativeFunction(this, &Transform_get_pixelBounds), createNativeFunction(this, &Transform_set_pixelBounds));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > As_flash_geom_Transform::alloc(ActionContext* context)
{
	return new Transform();
}

void As_flash_geom_Transform::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	if (args.size() < 1)
		return;

	Ref< FlashCharacterInstance > instance = args[0].getObject< FlashCharacterInstance >();
	if (instance)
	{
		Transform* t = checked_type_cast< Transform* >(self);
		t->setInstance(instance);
	}
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
