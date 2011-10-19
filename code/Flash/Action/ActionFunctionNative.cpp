#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunctionNative", ActionFunctionNative, ActionFunction)

ActionFunctionNative::ActionFunctionNative(ActionContext* context, INativeFunction* nativeFunction)
:	ActionFunction(context, "<native>")
,	m_nativeFunction(nativeFunction)
{
}

ActionValue ActionFunctionNative::call(ActionObject* self, const ActionValueArray& args)
{
	CallArgs fnc;
	fnc.context = getContext();
	fnc.self = self;
	fnc.args = args;

	if (m_nativeFunction)
		m_nativeFunction->call(fnc);

	return fnc.ret;
}

ActionValue ActionFunctionNative::call(ActionFrame* callerFrame, ActionObject* self)
{
	ActionValueStack& callerStack = callerFrame->getStack();
	int argCount = !callerStack.empty() ? int(callerStack.pop().getNumber()) : 0;

	CallArgs fnc;
	fnc.context = getContext();
	fnc.self = self;
	fnc.args = ActionValueArray(getContext()->getPool(), argCount);

	for (int i = 0; i < argCount; ++i)
		fnc.args[i] = callerStack.pop();

	if (m_nativeFunction)
		m_nativeFunction->call(fnc);

	return fnc.ret;
}

Ref< ActionFunctionNative > createPolymorphicFunction(
	ActionContext* context,
	ActionFunctionNative* fnptr_0,
	ActionFunctionNative* fnptr_1,
	ActionFunctionNative* fnptr_2,
	ActionFunctionNative* fnptr_3,
	ActionFunctionNative* fnptr_4
)
{
	return new ActionFunctionNative(
		context,
		new PolymorphicNativeFunction(
			fnptr_0,
			fnptr_1,
			fnptr_2,
			fnptr_3,
			fnptr_4
		)
	);
}

	}
}
