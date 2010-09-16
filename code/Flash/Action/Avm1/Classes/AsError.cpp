#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsError.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsError", AsError, ActionClass)

AsError::AsError()
:	ActionClass(L"Error")
{
	Ref< ActionObject > prototype = new ActionObject();

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
