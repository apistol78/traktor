#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/ColorTransform.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_ColorTransform.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_ColorTransform", As_flash_geom_ColorTransform, ActionClass)

As_flash_geom_ColorTransform::As_flash_geom_ColorTransform()
:	ActionClass("flash.geom.Transform")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->addProperty("alphaMultiplier", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_alphaMultiplier), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_alphaMultiplier));
	prototype->addProperty("alphaOffset", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_alphaOffset), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_alphaOffset));
	prototype->addProperty("blueMultiplier", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_blueMultiplier), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_blueMultiplier));
	prototype->addProperty("blueOffset", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_blueOffset), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_blueOffset));
	prototype->addProperty("greenMultiplier", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_greenMultiplier), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_greenMultiplier));
	prototype->addProperty("greenOffset", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_greenOffset), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_greenOffset));
	prototype->addProperty("redMultiplier", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_redMultiplier), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_redMultiplier));
	prototype->addProperty("redOffset", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_redOffset), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_redOffset));
	prototype->addProperty("rgb", createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_get_rgb), createNativeFunction(this, &As_flash_geom_ColorTransform::ColorTransform_set_rgb));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue As_flash_geom_ColorTransform::construct(ActionContext* context, const ActionValueArray& args)
{
	if (args.size() < 8)
		return ActionValue();

	SwfCxTransform transform;

	transform.red[0] = float(args[0].getNumberSafe());
	transform.green[0] = float(args[1].getNumberSafe());
	transform.blue[0] = float(args[2].getNumberSafe());
	transform.alpha[0] = float(args[3].getNumberSafe());
	transform.red[1] = float(args[4].getNumberSafe());
	transform.green[1] = float(args[5].getNumberSafe());
	transform.blue[1] = float(args[6].getNumberSafe());
	transform.alpha[1] = float(args[7].getNumberSafe());

	return ActionValue(new ColorTransform(transform));
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_alphaMultiplier(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().alpha[0]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_alphaMultiplier(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().alpha[0] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_alphaOffset(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().alpha[1]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_alphaOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().alpha[1] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_blueMultiplier(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().blue[0]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_blueMultiplier(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().blue[0] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_blueOffset(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().blue[1]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_blueOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().blue[1] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_greenMultiplier(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().green[0]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_greenMultiplier(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().green[0] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_greenOffset(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().green[1]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_greenOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().green[1] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_redMultiplier(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().red[0]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_redMultiplier(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().red[0] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_redOffset(ColorTransform* self) const
{
	return avm_number_t(self->getTransform().red[1]);
}

void As_flash_geom_ColorTransform::ColorTransform_set_redOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().red[1] = float(value);
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_rgb(ColorTransform* self) const
{
	return 0.0f;
}

void As_flash_geom_ColorTransform::ColorTransform_set_rgb(ColorTransform* self, avm_number_t value) const
{
}

	}
}
