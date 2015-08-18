#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionSuper.h"
#include "Flash/Action/Common/Classes/AsFunction.h"

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

ActionValue ActionFunctionNative::call(ActionObject* self, ActionObject* super, const ActionValueArray& args)
{
	T_ASSERT (!is_a< ActionSuper >(self));

	CallArgs fnc;
	fnc.context = getContext();
	fnc.self = self;
	fnc.super = super;
	fnc.args = args;

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

void trapInvalidCast()
{
	T_DEBUG(L"Trying to call object with incorrect type");
}

	}
}
