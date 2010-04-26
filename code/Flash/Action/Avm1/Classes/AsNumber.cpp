#include <limits>
#include "Core/Misc/String.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsNumber.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Classes/Number.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsNumber", AsNumber, ActionClass)

Ref< AsNumber > AsNumber::getInstance()
{
	static Ref< AsNumber > instance = 0;
	if (!instance)
	{
		instance = new AsNumber();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsNumber::AsNumber()
:	ActionClass(L"Number")
{
}

void AsNumber::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"MAX_VALUE", ActionValue(std::numeric_limits< avm_number_t >::max()));
	prototype->setMember(L"MIN_VALUE", ActionValue(std::numeric_limits< avm_number_t >::min()));
	prototype->setMember(L"NaN", ActionValue(std::numeric_limits< avm_number_t >::signaling_NaN()));
	prototype->setMember(L"NEGATIVE_INFINITY", ActionValue(-std::numeric_limits< avm_number_t >::infinity()));
	prototype->setMember(L"POSITIVE_INFINITY", ActionValue(std::numeric_limits< avm_number_t >::infinity()));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsNumber::Number_toString)));
	prototype->setMember(L"valueOf", ActionValue(createNativeFunction(this, &AsNumber::Number_valueOf)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsNumber::construct(ActionContext* context, const ActionValueArray& args)
{
	if (args.empty())
		return ActionValue(new Number(0.0));
	else
		return ActionValue(new Number(args[0].getNumberSafe()));
}

void AsNumber::Number_toString(CallArgs& ca)
{
	const Number* obj = checked_type_cast< const Number* >(ca.self);
	ca.ret = ActionValue(traktor::toString(obj->get()));
}

void AsNumber::Number_valueOf(CallArgs& ca)
{
	const Number* obj = checked_type_cast< const Number* >(ca.self);
	ca.ret = ActionValue(obj->get());
}

	}
}
