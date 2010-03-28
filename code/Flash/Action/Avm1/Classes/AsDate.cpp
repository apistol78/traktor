#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Date.h"
#include "Flash/Action/Avm1/Classes/AsDate.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsDate", AsDate, ActionClass)

Ref< AsDate > AsDate::getInstance()
{
	static Ref< AsDate > instance = 0;
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
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"getDate", ActionValue(createNativeFunction(this, &AsDate::Date_getDate)));
	prototype->setMember(L"getDay", ActionValue(createNativeFunction(this, &AsDate::Date_getDay)));
	prototype->setMember(L"getFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_getFullYear)));
	prototype->setMember(L"getHours", ActionValue(createNativeFunction(this, &AsDate::Date_getHours)));
	prototype->setMember(L"getMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_getMilliseconds)));
	prototype->setMember(L"getMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_getMinutes)));
	prototype->setMember(L"getMonth", ActionValue(createNativeFunction(this, &AsDate::Date_getMonth)));
	prototype->setMember(L"getSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_getSeconds)));
	prototype->setMember(L"getTime", ActionValue(createNativeFunction(this, &AsDate::Date_getTime)));
	prototype->setMember(L"getTimezoneOffset", ActionValue(createNativeFunction(this, &AsDate::Date_getTimezoneOffset)));
	prototype->setMember(L"getUTCDate", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCDate)));
	prototype->setMember(L"getUTCDay", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCDay)));
	prototype->setMember(L"getUTCFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCFullYear)));
	prototype->setMember(L"getUTCHours", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCHours)));
	prototype->setMember(L"getUTCMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCMilliseconds)));
	prototype->setMember(L"getUTCMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCMinutes)));
	prototype->setMember(L"getUTCMonth", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCMonth)));
	prototype->setMember(L"getUTCSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCSeconds)));
	prototype->setMember(L"getUTCYear", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCYear)));
	prototype->setMember(L"getYear", ActionValue(createNativeFunction(this, &AsDate::Date_getYear)));
	prototype->setMember(L"setDate", ActionValue(createNativeFunction(this, &AsDate::Date_setDate)));
	prototype->setMember(L"setFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_setFullYear)));
	prototype->setMember(L"setHours", ActionValue(createNativeFunction(this, &AsDate::Date_setHours)));
	prototype->setMember(L"setMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_setMilliseconds)));
	prototype->setMember(L"setMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_setMinutes)));
	prototype->setMember(L"setMonth", ActionValue(createNativeFunction(this, &AsDate::Date_setMonth)));
	prototype->setMember(L"setSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_setSeconds)));
	prototype->setMember(L"setTime", ActionValue(createNativeFunction(this, &AsDate::Date_setTime)));
	prototype->setMember(L"setUTCDate", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCDate)));
	prototype->setMember(L"setUTCFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCFullYear)));
	prototype->setMember(L"setUTCHours", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCHours)));
	prototype->setMember(L"setUTCMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCMilliseconds)));
	prototype->setMember(L"setUTCMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCMinutes)));
	prototype->setMember(L"setUTCMonth", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCMonth)));
	prototype->setMember(L"setUTCSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCSeconds)));
	prototype->setMember(L"setYear", ActionValue(createNativeFunction(this, &AsDate::Date_setYear)));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsDate::Date_toString)));
	prototype->setMember(L"UTC", ActionValue(createNativeFunction(this, &AsDate::Date_UTC)));
	prototype->setMember(L"valueOf", ActionValue(createNativeFunction(this, &AsDate::Date_valueOf)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsDate::construct(ActionContext* context, const args_t& args)
{
	return ActionValue(new Date());
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
