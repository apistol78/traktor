#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm1/ActionClass.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionClass", ActionClass, ActionFunction)

ActionClass::ActionClass(ActionContext* context, const std::string& className)
:	ActionFunction(context, className)
{
}

ActionValue ActionClass::call(ActionObject* self, const ActionValueArray& args)
{
	init(self, args);
	return ActionValue();
}

ActionValue ActionClass::call(ActionFrame* callerFrame, ActionObject* self)
{
	if (self)
	{
		ActionValueStack& callerStack = callerFrame->getStack();
		int32_t argCount = !callerStack.empty() ? int32_t(callerStack.pop().getNumber()) : 0;

		ActionValueArray args(getContext()->getPool(), argCount);
		for (int32_t i = 0; i < argCount; ++i)
			args[i] = callerStack.pop();

		init(self, args);
	}
	return ActionValue();
}

	}
}
