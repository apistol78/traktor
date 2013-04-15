#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/ColorTransform.h"
#include "Flash/Action/Avm1/Classes/As_flash_geom_ColorTransform.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_ColorTransform", As_flash_geom_ColorTransform, ActionClass)

As_flash_geom_ColorTransform::As_flash_geom_ColorTransform(ActionContext* context)
:	ActionClass(context, "flash.geom.ColorTransform")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->addProperty("alphaMultiplier", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_alphaMultiplier), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_alphaMultiplier));
	prototype->addProperty("alphaOffset", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_alphaOffset), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_alphaOffset));
	prototype->addProperty("blueMultiplier", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_blueMultiplier), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_blueMultiplier));
	prototype->addProperty("blueOffset", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_blueOffset), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_blueOffset));
	prototype->addProperty("greenMultiplier", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_greenMultiplier), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_greenMultiplier));
	prototype->addProperty("greenOffset", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_greenOffset), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_greenOffset));
	prototype->addProperty("redMultiplier", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_redMultiplier), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_redMultiplier));
	prototype->addProperty("redOffset", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_redOffset), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_redOffset));
	prototype->addProperty("rgb", createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_get_rgb), createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_set_rgb));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &As_flash_geom_ColorTransform::ColorTransform_toString)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_ColorTransform::initialize(ActionObject* self)
{
}

void As_flash_geom_ColorTransform::construct(ActionObject* self, const ActionValueArray& args)
{
	SwfCxTransform transform;
	if (args.size() >= 8)
	{
		transform.red[0]   = float(args[0].getNumber());
		transform.green[0] = float(args[1].getNumber());
		transform.blue[0]  = float(args[2].getNumber());
		transform.alpha[0] = float(args[3].getNumber());
		transform.red[1]   = float(args[4].getNumber()) / 255.0f;
		transform.green[1] = float(args[5].getNumber()) / 255.0f;
		transform.blue[1]  = float(args[6].getNumber()) / 255.0f;
		transform.alpha[1] = float(args[7].getNumber()) / 255.0f;
	}
	self->setRelay(new ColorTransform(transform));
}

ActionValue As_flash_geom_ColorTransform::xplicit(const ActionValueArray& args)
{
	return ActionValue();
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
	return avm_number_t(self->getTransform().alpha[1] * 255.0f);
}

void As_flash_geom_ColorTransform::ColorTransform_set_alphaOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().alpha[1] = float(value) / 255.0f;
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
	return avm_number_t(self->getTransform().blue[1] * 255.0f);
}

void As_flash_geom_ColorTransform::ColorTransform_set_blueOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().blue[1] = float(value) / 255.0f;
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
	return avm_number_t(self->getTransform().green[1] * 255.0f);
}

void As_flash_geom_ColorTransform::ColorTransform_set_greenOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().green[1] = float(value) / 255.0f;
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
	return avm_number_t(self->getTransform().red[1] * 255.0f);
}

void As_flash_geom_ColorTransform::ColorTransform_set_redOffset(ColorTransform* self, avm_number_t value) const
{
	self->getTransform().red[1] = float(value) / 255.0f;
}

avm_number_t As_flash_geom_ColorTransform::ColorTransform_get_rgb(ColorTransform* self) const
{
	T_IF_VERBOSE(
		log::warning << L"ColorTransform::get_rgb not implemented" << Endl;
	)
	return 0.0f;
}

void As_flash_geom_ColorTransform::ColorTransform_set_rgb(ColorTransform* self, avm_number_t value) const
{
	T_IF_VERBOSE(
		log::warning << L"ColorTransform::set_rgb not implemented" << Endl;
	)
}

std::wstring As_flash_geom_ColorTransform::ColorTransform_toString(const ColorTransform* self) const
{
	return L"{redMultiplier=0, greenMultiplier=0, blueMultiplier=0, alphaMultiplier=0, redOffset=0, greenOffset=0, blueOffset=0, alphaOffset=0}";
}

	}
}
