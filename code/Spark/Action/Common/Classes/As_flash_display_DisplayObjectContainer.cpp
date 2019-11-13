#include "Spark/Action/Common/Classes/As_flash_display_DisplayObjectContainer.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.As_flash_display_DisplayObjectContainer", As_flash_display_DisplayObjectContainer, ActionClass)

As_flash_display_DisplayObjectContainer::As_flash_display_DisplayObjectContainer(ActionContext* context)
:	ActionClass(context, "flash.display.DisplayObjectContainer")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_display_DisplayObjectContainer::initialize(ActionObject* self)
{
}

void As_flash_display_DisplayObjectContainer::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue As_flash_display_DisplayObjectContainer::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
