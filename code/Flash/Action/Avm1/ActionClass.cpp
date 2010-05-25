#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm1/ActionClass.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionClass", ActionClass, ActionFunction)

ActionClass::ActionClass(const std::wstring& name)
:	ActionFunction(name)
{
}

ActionValue ActionClass::call(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	return construct(context, args);
}

ActionValue ActionClass::call(ActionFrame* callerFrame, ActionObject* self)
{
	ActionValueStack& callerStack = callerFrame->getStack();
	int32_t argCount = !callerStack.empty() ? int32_t(callerStack.pop().getNumber()) : 0;

	ActionValueArray args(callerFrame->getContext()->getPool(), argCount);
	for (int32_t i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	return construct(callerFrame->getContext(), args);
}

	}
}
