#include "Flash/FlashTextFormat.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsTextFormat.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextFormat", AsTextFormat, ActionClass)

AsTextFormat::AsTextFormat()
:	ActionClass("TextFormat")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsTextFormat::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue(new FlashTextFormat());
}

	}
}
