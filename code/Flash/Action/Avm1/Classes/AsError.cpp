#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsError.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsError", AsError, ActionClass)

AsError::AsError(ActionContext* context)
:	ActionClass(context, "Error")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("message", ActionValue(L"Error"));
	prototype->setMember("name", ActionValue(L"Error"));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsError::Error_toString)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsError::init(ActionObject* self, const ActionValueArray& args)
{
	if (args.size() > 0)
		self->setMember("message", args[0]);
}

void AsError::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void AsError::Error_toString(CallArgs& ca)
{
	ActionValue messageValue;
	if (ca.self && ca.self->getMember("message", messageValue))
		ca.ret = messageValue;
	else
		ca.ret = ActionValue("Error");
}

	}
}
