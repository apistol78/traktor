#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionClass", ActionClass, ActionFunction)

ActionClass::ActionClass(ActionContext* context, const char* className)
:	ActionFunction(context, className)
{
}

ActionValue ActionClass::call(ActionObject* self, ActionObject* super, const ActionValueArray& args)
{
	if (self)
	{
		construct(self, args);
		return ActionValue(self);
	}
	else
		return xplicit(args);
}

	}
}
