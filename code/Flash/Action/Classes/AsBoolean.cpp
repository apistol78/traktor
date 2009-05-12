#include "Flash/Action/Classes/AsBoolean.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionBoolean.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsBoolean", AsBoolean, ActionClass)

AsBoolean* AsBoolean::getInstance()
{
	static AsBoolean* instance = 0;
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
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsBoolean::Boolean_toString));
	prototype->setMember(L"valueOf", createNativeFunctionValue(this, &AsBoolean::Boolean_valueOf));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsBoolean::construct(ActionContext* context, const args_t& args)
{
	if (args.empty())
		return ActionValue::fromObject(gc_new< ActionBoolean >(false));
	else
		return ActionValue::fromObject(gc_new< ActionBoolean >(args[0].getBooleanSafe()));
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
