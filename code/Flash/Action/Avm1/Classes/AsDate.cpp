#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Date.h"
#include "Flash/Action/Avm1/Classes/AsDate.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsDate", AsDate, ActionClass)

AsDate::AsDate()
:	ActionClass("Date")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("getDate", ActionValue(createNativeFunction(this, &AsDate::Date_getDate)));
	prototype->setMember("getDay", ActionValue(createNativeFunction(this, &AsDate::Date_getDay)));
	prototype->setMember("getFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_getFullYear)));
	prototype->setMember("getHours", ActionValue(createNativeFunction(this, &AsDate::Date_getHours)));
	prototype->setMember("getMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_getMilliseconds)));
	prototype->setMember("getMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_getMinutes)));
	prototype->setMember("getMonth", ActionValue(createNativeFunction(this, &AsDate::Date_getMonth)));
	prototype->setMember("getSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_getSeconds)));
	prototype->setMember("getTime", ActionValue(createNativeFunction(this, &AsDate::Date_getTime)));
	prototype->setMember("getTimezoneOffset", ActionValue(createNativeFunction(this, &AsDate::Date_getTimezoneOffset)));
	prototype->setMember("getUTCDate", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCDate)));
	prototype->setMember("getUTCDay", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCDay)));
	prototype->setMember("getUTCFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCFullYear)));
	prototype->setMember("getUTCHours", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCHours)));
	prototype->setMember("getUTCMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCMilliseconds)));
	prototype->setMember("getUTCMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCMinutes)));
	prototype->setMember("getUTCMonth", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCMonth)));
	prototype->setMember("getUTCSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCSeconds)));
	prototype->setMember("getUTCYear", ActionValue(createNativeFunction(this, &AsDate::Date_getUTCYear)));
	prototype->setMember("getYear", ActionValue(createNativeFunction(this, &AsDate::Date_getYear)));
	prototype->setMember("setDate", ActionValue(createNativeFunction(this, &AsDate::Date_setDate)));
	prototype->setMember("setFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_setFullYear)));
	prototype->setMember("setHours", ActionValue(createNativeFunction(this, &AsDate::Date_setHours)));
	prototype->setMember("setMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_setMilliseconds)));
	prototype->setMember("setMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_setMinutes)));
	prototype->setMember("setMonth", ActionValue(createNativeFunction(this, &AsDate::Date_setMonth)));
	prototype->setMember("setSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_setSeconds)));
	prototype->setMember("setTime", ActionValue(createNativeFunction(this, &AsDate::Date_setTime)));
	prototype->setMember("setUTCDate", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCDate)));
	prototype->setMember("setUTCFullYear", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCFullYear)));
	prototype->setMember("setUTCHours", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCHours)));
	prototype->setMember("setUTCMilliseconds", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCMilliseconds)));
	prototype->setMember("setUTCMinutes", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCMinutes)));
	prototype->setMember("setUTCMonth", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCMonth)));
	prototype->setMember("setUTCSeconds", ActionValue(createNativeFunction(this, &AsDate::Date_setUTCSeconds)));
	prototype->setMember("setYear", ActionValue(createNativeFunction(this, &AsDate::Date_setYear)));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &AsDate::Date_toString)));
	prototype->setMember("UTC", ActionValue(createNativeFunction(this, &AsDate::Date_UTC)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(this, &AsDate::Date_valueOf)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsDate::construct(ActionContext* context, const ActionValueArray& args)
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
