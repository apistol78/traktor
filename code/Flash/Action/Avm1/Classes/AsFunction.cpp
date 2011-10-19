#include "Core/Log/Log.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Action/Avm1/Classes/AsFunction.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsFunction", AsFunction, ActionClass)

AsFunction::AsFunction()
:	ActionClass("Function")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("apply", ActionValue(createNativeFunction(this, &AsFunction::Function_apply)));
	prototype->setMember("call", ActionValue(createNativeFunction(this, &AsFunction::Function_call)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsFunction::alloc(ActionContext* context)
{
	return new ActionObject("Function");
}

void AsFunction::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
}

void AsFunction::Function_apply(CallArgs& ca)
{
	if (ca.args.size() < 2)
	{
		log::error << L"Function.apply, incorrect number of arguments" << Endl;
		return;
	}

	ActionFunction* function = dynamic_type_cast< ActionFunction* >(ca.self);
	if (!function)
	{
		log::error << L"Function.apply, invalid object" << Endl;
		return;
	}

	Ref< ActionObject > self = ca.args[0].getObject();
	Ref< Array > args = ca.args[1].getObject< Array >();

	ActionFrame frame(
		ca.context,
		ca.self,
		0,
		0,
		0,
		function
	);

	ActionValueStack& stack = frame.getStack();
	if (args)
	{
		const std::vector< ActionValue >& argValues = args->getValues();
		for (size_t i = 0; i < argValues.size(); ++i)
			stack.push(argValues[i]);
		stack.push(ActionValue(avm_number_t(argValues.size())));
	}
	else
		stack.push(ActionValue(avm_number_t(0)));

	function->call(
		&frame,
		self
	);

	if (stack.depth() > 0)
		ca.ret = stack.pop();
}

void AsFunction::Function_call(CallArgs& ca)
{
	if (ca.args.size() < 1)
	{
		log::error << L"Function.call, incorrect number of arguments" << Endl;
		return;
	}

	ActionFunction* function = dynamic_type_cast< ActionFunction* >(ca.self);
	if (!function)
	{
		log::error << L"Function.apply, invalid object" << Endl;
		return;
	}

	ActionObject* self = ca.args[0].getObject();

	ActionFrame frame(
		ca.context,
		ca.self,
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
		&frame,
		self
	);

	if (stack.depth() > 0)
		ca.ret = stack.pop();
}

	}
}
