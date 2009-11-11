#include <limits>
#include "Flash/Action/Classes/AsString.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionString.h"
#include "Flash/Action/ActionArray.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsString", AsString, ActionClass)

Ref< AsString > AsString::getInstance()
{
	static AsString* instance = 0;
	if (!instance)
	{
		instance = new AsString();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsString::AsString()
:	ActionClass(L"String")
{
}

void AsString::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"charAt", createNativeFunctionValue(this, &AsString::String_charAt));
	prototype->setMember(L"charCodeAt", createNativeFunctionValue(this, &AsString::String_charCodeAt));
	prototype->setMember(L"concat", createNativeFunctionValue(this, &AsString::String_concat));
	prototype->setMember(L"fromCharCode", createNativeFunctionValue(this, &AsString::String_fromCharCode));
	prototype->setMember(L"indexOf", createNativeFunctionValue(this, &AsString::String_indexOf));
	prototype->setMember(L"lastIndexOf", createNativeFunctionValue(this, &AsString::String_lastIndexOf));
	prototype->setMember(L"slice", createNativeFunctionValue(this, &AsString::String_slice));
	prototype->setMember(L"split", createNativeFunctionValue(this, &AsString::String_split));
	prototype->setMember(L"substr", createNativeFunctionValue(this, &AsString::String_substr));
	prototype->setMember(L"substring", createNativeFunctionValue(this, &AsString::String_substring));
	prototype->setMember(L"toLowerCase", createNativeFunctionValue(this, &AsString::String_toLowerCase));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsString::String_toString));
	prototype->setMember(L"toUpperCase", createNativeFunctionValue(this, &AsString::String_toUpperCase));
	prototype->setMember(L"valueOf", createNativeFunctionValue(this, &AsString::String_valueOf));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsString::construct(ActionContext* context, const args_t& args)
{
	if (args.size() > 0)
		return ActionValue::fromObject(gc_new< ActionString >(args[0].getString()));
	else
		return ActionValue::fromObject(gc_new< ActionString >());
}

void AsString::String_charAt(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());

	if (index < st.length())
		ca.ret = ActionValue::fromObject(gc_new< ActionString >(st[index]));
	else
		ca.ret = ActionValue::fromObject(gc_new< ActionString >());
}

void AsString::String_charCodeAt(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());

	if (index < st.length())
		ca.ret = ActionValue(double(st[index]));
	else
		ca.ret = ActionValue(std::numeric_limits< double >::signaling_NaN());
}

void AsString::String_concat(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);

	StringOutputStream ss;
	ss << self->get();

	for (std::vector< ActionValue >::const_iterator i = ca.args.begin(); i != ca.args.end(); ++i)
		ss << i->getStringSafe();

	ca.ret = ActionValue(ss.str());
}

void AsString::String_fromCharCode(CallArgs& ca)
{
	wchar_t charCode = wchar_t(ca.args[0].getNumberSafe());
	ca.ret = ActionValue::fromObject(gc_new< ActionString >(charCode));
}

void AsString::String_indexOf(CallArgs& ca)
{
}

void AsString::String_lastIndexOf(CallArgs& ca)
{
}

void AsString::String_slice(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();

	if (ca.args.size() >= 2)
	{
		uint32_t start = uint32_t(ca.args[0].getNumberSafe());
		uint32_t end = uint32_t(ca.args[1].getNumberSafe());
		ca.ret = ActionValue(st.substr(start, end - start));
	}
	else if (ca.args.size() >= 1)
	{
		uint32_t start = uint32_t(ca.args[0].getNumberSafe());
		ca.ret = ActionValue(st.substr(start));
	}
}

void AsString::String_split(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();

	std::vector< std::wstring > words;
	if (ca.args.size() >= 2)
	{
		std::wstring delim = ca.args[0].getStringSafe();
		uint32_t limit = uint32_t(ca.args[1].getNumberSafe());
		Split< std::wstring >::word(st, delim, words, limit);
	}
	else if (ca.args.size() >= 1)
	{
		std::wstring delim = ca.args[0].getStringSafe();
		Split< std::wstring >::word(st, delim, words);
	}

	Ref< ActionArray > arr = gc_new< ActionArray >();
	for (std::vector< std::wstring >::const_iterator i = words.begin(); i != words.end(); ++i)
		arr->push(ActionValue(*i));

	ca.ret = ActionValue::fromObject(arr);
}

void AsString::String_substr(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());
	uint32_t count = uint32_t(ca.args[1].getNumberSafe());

	ca.ret = ActionValue(st.substr(index, count));
}

void AsString::String_substring(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t start = uint32_t(ca.args[0].getNumberSafe());
	uint32_t end = uint32_t(ca.args[1].getNumberSafe());

	ca.ret = ActionValue(st.substr(start, end - start));
}

void AsString::String_toLowerCase(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();
	ca.ret = ActionValue::fromObject(gc_new< ActionString >(toLower(st)));
}

void AsString::String_toString(CallArgs& ca)
{
	ca.ret = ActionValue::fromObject(ca.self);
}

void AsString::String_toUpperCase(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	const std::wstring& st = self->get();
	ca.ret = ActionValue::fromObject(gc_new< ActionString >(toUpper(st)));
}

void AsString::String_valueOf(CallArgs& ca)
{
	Ref< ActionString > self = checked_type_cast< ActionString* >(ca.self);
	ca.ret = ActionValue(self->get());
}

	}
}
