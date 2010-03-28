#include "Core/Log/Log.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsFunction", AsFunction, ActionClass)

Ref< AsFunction > AsFunction::getInstance()
{
	static Ref< AsFunction > instance;
	if (!instance)
	{
		instance = new AsFunction();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsFunction::AsFunction()
:	ActionClass(L"Function")
{
}

void AsFunction::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"call", ActionValue(createNativeFunction(this, &AsFunction::Function_call)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsFunction::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsFunction::Function_call(CallArgs& ca)
{
	if (ca.args.size() < 1)
	{
		log::error << L"Function.call, incorrect number of arguments" << Endl;
		return;
	}

	ActionFunction* function = checked_type_cast< ActionFunction*, false >(ca.self);

	Ref< ActionObject > self = ca.args[0].getObjectSafe();

	ActionFrame frame(
		ca.context,
		ca.self,
		0,
		0,
		0,
		0,
		function
	);

	ActionValueStack& stack = frame.getStack();
	for (size_t i = 1; i < ca.args.size(); ++i)
		stack.push(ca.args[i]);

	stack.push(ActionValue(avm_number_t(ca.args.size() - 1)));

	function->call(
		ca.vm,
		&frame,
		self
	);

	if (stack.depth() > 0)
		ca.ret = stack.pop();
}

	}
}
