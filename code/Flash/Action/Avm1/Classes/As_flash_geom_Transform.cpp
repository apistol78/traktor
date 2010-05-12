#include "Flash/FlashCharacterInstance.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/ColorTransform.h"
#include "Flash/Action/Classes/Transform.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_Transform.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Transform", As_flash_geom_Transform, ActionClass)

Ref< As_flash_geom_Transform > As_flash_geom_Transform::getInstance()
{
	static Ref< As_flash_geom_Transform > instance;
	if (!instance)
	{
		instance = new As_flash_geom_Transform();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

As_flash_geom_Transform::As_flash_geom_Transform()
:	ActionClass(L"Transform")
{
}

void As_flash_geom_Transform::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->addProperty(L"colorTransform", createNativeFunction(this, &As_flash_geom_Transform::Transform_get_colorTransform), createNativeFunction(this, &As_flash_geom_Transform::Transform_set_colorTransform));
	//prototype->addProperty(L"concatenatedColorTransform", createNativeFunction(this, &Transform_get_concatenatedColorTransform), 0);
	//prototype->addProperty(L"concatenatedMatrix", createNativeFunction(this, &Transform_get_concatenatedMatrix), 0);
	//prototype->addProperty(L"matrix", createNativeFunction(this, &Transform_get_matrix), createNativeFunction(this, &Transform_set_matrix));
	//prototype->addProperty(L"pixelBounds", createNativeFunction(this, &Transform_get_pixelBounds), createNativeFunction(this, &Transform_set_pixelBounds));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue As_flash_geom_Transform::construct(ActionContext* context, const ActionValueArray& args)
{
	if (args.size() >= 1)
	{
		Ref< FlashCharacterInstance > instance = args[0].getObjectSafe< FlashCharacterInstance >();
		if (instance)
			return ActionValue(new Transform(instance));
	}
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
