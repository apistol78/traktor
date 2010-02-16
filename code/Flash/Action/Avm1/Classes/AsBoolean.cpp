#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionBoolean.h"
#include "Flash/Action/Avm1/Classes/AsBoolean.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsBoolean", AsBoolean, ActionClass)

Ref< AsBoolean > AsBoolean::getInstance()
{
	static Ref< AsBoolean > instance = 0;
	if (!instance)
	{
		instance = new AsBoolean();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsBoolean::AsBoolean()
:	ActionClass(L"Boolean")
{
}

void AsBoolean::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsBoolean::Boolean_toString));
	prototype->setMember(L"valueOf", createNativeFunctionValue(this, &AsBoolean::Boolean_valueOf));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsBoolean::construct(ActionContext* context, const args_t& args)
{
	if (args.empty())
		return ActionValue(new ActionBoolean(false));
	else
		return ActionValue(new ActionBoolean(args[0].getBooleanSafe()));
}

void AsBoolean::Boolean_toString(CallArgs& ca)
{
	const ActionBoolean* obj = checked_type_cast< const ActionBoolean* >(ca.self);
	ca.ret = ActionValue(obj->get() ? L"true" : L"false");
}

void AsBoolean::Boolean_valueOf(CallArgs& ca)
{
	const ActionBoolean* obj = checked_type_cast< const ActionBoolean* >(ca.self);
	ca.ret = ActionValue(obj->get());
}

	}
}
