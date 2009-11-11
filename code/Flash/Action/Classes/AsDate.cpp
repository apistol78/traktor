#include "Flash/Action/Classes/AsDate.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionDate.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsDate", AsDate, ActionClass)

Ref< AsDate > AsDate::getInstance()
{
	static AsDate* instance = 0;
	if (!instance)
	{
		instance = new AsDate();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsDate::AsDate()
:	ActionClass(L"Date")
{
}

void AsDate::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"getDate", createNativeFunctionValue(this, &AsDate::Date_getDate));
	prototype->setMember(L"getDay", createNativeFunctionValue(this, &AsDate::Date_getDay));
	prototype->setMember(L"getFullYear", createNativeFunctionValue(this, &AsDate::Date_getFullYear));
	prototype->setMember(L"getHours", createNativeFunctionValue(this, &AsDate::Date_getHours));
	prototype->setMember(L"getMilliseconds", createNativeFunctionValue(this, &AsDate::Date_getMilliseconds));
	prototype->setMember(L"getMinutes", createNativeFunctionValue(this, &AsDate::Date_getMinutes));
	prototype->setMember(L"getMonth", createNativeFunctionValue(this, &AsDate::Date_getMonth));
	prototype->setMember(L"getSeconds", createNativeFunctionValue(this, &AsDate::Date_getSeconds));
	prototype->setMember(L"getTime", createNativeFunctionValue(this, &AsDate::Date_getTime));
	prototype->setMember(L"getTimezoneOffset", createNativeFunctionValue(this, &AsDate::Date_getTimezoneOffset));
	prototype->setMember(L"getUTCDate", createNativeFunctionValue(this, &AsDate::Date_getUTCDate));
	prototype->setMember(L"getUTCDay", createNativeFunctionValue(this, &AsDate::Date_getUTCDay));
	prototype->setMember(L"getUTCFullYear", createNativeFunctionValue(this, &AsDate::Date_getUTCFullYear));
	prototype->setMember(L"getUTCHours", createNativeFunctionValue(this, &AsDate::Date_getUTCHours));
	prototype->setMember(L"getUTCMilliseconds", createNativeFunctionValue(this, &AsDate::Date_getUTCMilliseconds));
	prototype->setMember(L"getUTCMinutes", createNativeFunctionValue(this, &AsDate::Date_getUTCMinutes));
	prototype->setMember(L"getUTCMonth", createNativeFunctionValue(this, &AsDate::Date_getUTCMonth));
	prototype->setMember(L"getUTCSeconds", createNativeFunctionValue(this, &AsDate::Date_getUTCSeconds));
	prototype->setMember(L"getUTCYear", createNativeFunctionValue(this, &AsDate::Date_getUTCYear));
	prototype->setMember(L"getYear", createNativeFunctionValue(this, &AsDate::Date_getYear));
	prototype->setMember(L"setDate", createNativeFunctionValue(this, &AsDate::Date_setDate));
	prototype->setMember(L"setFullYear", createNativeFunctionValue(this, &AsDate::Date_setFullYear));
	prototype->setMember(L"setHours", createNativeFunctionValue(this, &AsDate::Date_setHours));
	prototype->setMember(L"setMilliseconds", createNativeFunctionValue(this, &AsDate::Date_setMilliseconds));
	prototype->setMember(L"setMinutes", createNativeFunctionValue(this, &AsDate::Date_setMinutes));
	prototype->setMember(L"setMonth", createNativeFunctionValue(this, &AsDate::Date_setMonth));
	prototype->setMember(L"setSeconds", createNativeFunctionValue(this, &AsDate::Date_setSeconds));
	prototype->setMember(L"setTime", createNativeFunctionValue(this, &AsDate::Date_setTime));
	prototype->setMember(L"setUTCDate", createNativeFunctionValue(this, &AsDate::Date_setUTCDate));
	prototype->setMember(L"setUTCFullYear", createNativeFunctionValue(this, &AsDate::Date_setUTCFullYear));
	prototype->setMember(L"setUTCHours", createNativeFunctionValue(this, &AsDate::Date_setUTCHours));
	prototype->setMember(L"setUTCMilliseconds", createNativeFunctionValue(this, &AsDate::Date_setUTCMilliseconds));
	prototype->setMember(L"setUTCMinutes", createNativeFunctionValue(this, &AsDate::Date_setUTCMinutes));
	prototype->setMember(L"setUTCMonth", createNativeFunctionValue(this, &AsDate::Date_setUTCMonth));
	prototype->setMember(L"setUTCSeconds", createNativeFunctionValue(this, &AsDate::Date_setUTCSeconds));
	prototype->setMember(L"setYear", createNativeFunctionValue(this, &AsDate::Date_setYear));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsDate::Date_toString));
	prototype->setMember(L"UTC", createNativeFunctionValue(this, &AsDate::Date_UTC));
	prototype->setMember(L"valueOf", createNativeFunctionValue(this, &AsDate::Date_valueOf));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsDate::construct(ActionContext* context, const args_t& args)
{
	return ActionValue::fromObject(gc_new< ActionDate >());
}

void AsDate::Date_getDate(CallArgs& ca)
{
}

void AsDate::Date_getDay(CallArgs& ca)
{
}

void AsDate::Date_getFullYear(CallArgs& ca)
{
}

void AsDate::Date_getHours(CallArgs& ca)
{
}

void AsDate::Date_getMilliseconds(CallArgs& ca)
{
}

void AsDate::Date_getMinutes(CallArgs& ca)
{
}

void AsDate::Date_getMonth(CallArgs& ca)
{
}

void AsDate::Date_getSeconds(CallArgs& ca)
{
}

void AsDate::Date_getTime(CallArgs& ca)
{
}

void AsDate::Date_getTimezoneOffset(CallArgs& ca)
{
}

void AsDate::Date_getUTCDate(CallArgs& ca)
{
}

void AsDate::Date_getUTCDay(CallArgs& ca)
{
}

void AsDate::Date_getUTCFullYear(CallArgs& ca)
{
}

void AsDate::Date_getUTCHours(CallArgs& ca)
{
}

void AsDate::Date_getUTCMilliseconds(CallArgs& ca)
{
}

void AsDate::Date_getUTCMinutes(CallArgs& ca)
{
}

void AsDate::Date_getUTCMonth(CallArgs& ca)
{
}

void AsDate::Date_getUTCSeconds(CallArgs& ca)
{
}

void AsDate::Date_getUTCYear(CallArgs& ca)
{
}

void AsDate::Date_getYear(CallArgs& ca)
{
}

void AsDate::Date_setDate(CallArgs& ca)
{
}

void AsDate::Date_setFullYear(CallArgs& ca)
{
}

void AsDate::Date_setHours(CallArgs& ca)
{
}

void AsDate::Date_setMilliseconds(CallArgs& ca)
{
}

void AsDate::Date_setMinutes(CallArgs& ca)
{
}

void AsDate::Date_setMonth(CallArgs& ca)
{
}

void AsDate::Date_setSeconds(CallArgs& ca)
{
}

void AsDate::Date_setTime(CallArgs& ca)
{
}

void AsDate::Date_setUTCDate(CallArgs& ca)
{
}

void AsDate::Date_setUTCFullYear(CallArgs& ca)
{
}

void AsDate::Date_setUTCHours(CallArgs& ca)
{
}

void AsDate::Date_setUTCMilliseconds(CallArgs& ca)
{
}

void AsDate::Date_setUTCMinutes(CallArgs& ca)
{
}

void AsDate::Date_setUTCMonth(CallArgs& ca)
{
}

void AsDate::Date_setUTCSeconds(CallArgs& ca)
{
}

void AsDate::Date_setYear(CallArgs& ca)
{
}

void AsDate::Date_toString(CallArgs& ca)
{
}

void AsDate::Date_UTC(CallArgs& ca)
{
}

void AsDate::Date_valueOf(CallArgs& ca)
{
}

	}
}
