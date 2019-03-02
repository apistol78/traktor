#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueStack.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunction", ActionFunction, ActionObject)

ActionFunction::ActionFunction(ActionContext* context, const char* name)
:	ActionObject(context, "Function")
#if defined(_DEBUG)
,	m_name(name)
#endif
{
}

ActionValue ActionFunction::call(ActionFrame* callerFrame, ActionObject* self, ActionObject* super)
{
	T_ASSERT (callerFrame);

	ActionValueStack& callerStack = callerFrame->getStack();

	int32_t argCount = !callerStack.empty() ? callerStack.pop().getInteger() : 0;
	if (argCount < 0)
		argCount = 0;

	ActionValueArray args(callerFrame->getContext()->getPool(), argCount);
	for (int32_t i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	return call(self, super, args);
}

ActionValue ActionFunction::call(ActionFrame* callerFrame, ActionObject* self)
{
	T_ASSERT (callerFrame);

	ActionValueStack& callerStack = callerFrame->getStack();

	int32_t argCount = !callerStack.empty() ? callerStack.pop().getInteger() : 0;
	if (argCount < 0)
		argCount = 0;

	ActionValueArray args(callerFrame->getContext()->getPool(), argCount);
	for (int32_t i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	return call(self, args);
}

ActionValue ActionFunction::call(ActionFrame* callerFrame)
{
	T_ASSERT (callerFrame);

	ActionValueStack& callerStack = callerFrame->getStack();

	int32_t argCount = !callerStack.empty() ? callerStack.pop().getInteger() : 0;
	if (argCount < 0)
		argCount = 0;

	ActionValueArray args(callerFrame->getContext()->getPool(), argCount);
	for (int32_t i = 0; i < argCount; ++i)
		args[i] = callerStack.pop();

	return call(args);
}

	}
}
