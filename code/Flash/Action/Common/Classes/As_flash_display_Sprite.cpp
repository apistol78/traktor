#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_flash_display_Sprite.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_display_Sprite", As_flash_display_Sprite, ActionClass)

As_flash_display_Sprite::As_flash_display_Sprite(ActionContext* context)
:	ActionClass(context, "flash.display.Sprite")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_display_Sprite::initialize(ActionObject* self)
{
}

void As_flash_display_Sprite::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue As_flash_display_Sprite::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
