#include "Flash/Action/Classes/AsError.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionDate.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsError", AsError, ActionClass)

Ref< AsError > AsError::getInstance()
{
	static AsError* instance = 0;
	if (!instance)
	{
		instance = new AsError();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsError::AsError()
:	ActionClass(L"Error")
{
}

void AsError::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"message", ActionValue(L""));
	prototype->setMember(L"name", ActionValue(L""));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsError::construct(ActionContext* context, const args_t& args)
{
	Ref< ActionObject > errorObject = new ActionObject(this);

	if (args.size() > 0)
		errorObject->setMember(L"message", args[0]);

	return ActionValue::fromObject(errorObject);
}

	}
}
