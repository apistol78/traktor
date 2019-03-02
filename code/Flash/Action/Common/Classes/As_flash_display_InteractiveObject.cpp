#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_flash_display_InteractiveObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_display_InteractiveObject", As_flash_display_InteractiveObject, ActionClass)

As_flash_display_InteractiveObject::As_flash_display_InteractiveObject(ActionContext* context)
:	ActionClass(context, "flash.display.InteractiveObject")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_display_InteractiveObject::initialize(ActionObject* self)
{
}

void As_flash_display_InteractiveObject::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue As_flash_display_InteractiveObject::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
