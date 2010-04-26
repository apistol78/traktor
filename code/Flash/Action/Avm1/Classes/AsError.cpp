#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsError.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsError", AsError, ActionClass)

Ref< AsError > AsError::getInstance()
{
	static Ref< AsError > instance = 0;
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

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"message", ActionValue(L""));
	prototype->setMember(L"name", ActionValue(L""));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsError::construct(ActionContext* context, const ActionValueArray& args)
{
	Ref< ActionObject > errorObject = new ActionObject(this);

	if (args.size() > 0)
		errorObject->setMember(L"message", args[0]);

	return ActionValue(errorObject);
}

	}
}
