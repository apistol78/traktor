#include <limits>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/String.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Action/Avm1/Classes/AsString.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsString", AsString, ActionClass)

AsString::AsString()
:	ActionClass(L"String")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"charAt", ActionValue(createNativeFunction(this, &AsString::String_charAt)));
	prototype->setMember(L"charCodeAt", ActionValue(createNativeFunction(this, &AsString::String_charCodeAt)));
	prototype->setMember(L"concat", ActionValue(createNativeFunction(this, &AsString::String_concat)));
	prototype->setMember(L"fromCharCode", ActionValue(createNativeFunction(this, &AsString::String_fromCharCode)));
	prototype->setMember(L"indexOf", ActionValue(createNativeFunction(this, &AsString::String_indexOf)));
	prototype->setMember(L"lastIndexOf", ActionValue(createNativeFunction(this, &AsString::String_lastIndexOf)));
	prototype->setMember(L"slice", ActionValue(createNativeFunction(this, &AsString::String_slice)));
	prototype->setMember(L"split", ActionValue(createNativeFunction(this, &AsString::String_split)));
	prototype->setMember(L"substr", ActionValue(createNativeFunction(this, &AsString::String_substr)));
	prototype->setMember(L"substring", ActionValue(createNativeFunction(this, &AsString::String_substring)));
	prototype->setMember(L"toLowerCase", ActionValue(createNativeFunction(this, &AsString::String_toLowerCase)));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsString::String_toString)));
	prototype->setMember(L"toUpperCase", ActionValue(createNativeFunction(this, &AsString::String_toUpperCase)));
	prototype->setMember(L"valueOf", ActionValue(createNativeFunction(this, &AsString::String_valueOf)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsString::construct(ActionContext* context, const ActionValueArray& args)
{
	if (args.size() > 0)
		return ActionValue(new String(args[0].getString()));
	else
		return ActionValue(new String());
}

void AsString::String_charAt(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());

	if (index < st.length())
		ca.ret = ActionValue(new String(st[index]));
	else
		ca.ret = ActionValue(new String());
}

void AsString::String_charCodeAt(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());

	if (index < st.length())
		ca.ret = ActionValue(avm_number_t(st[index]));
	else
		ca.ret = ActionValue(std::numeric_limits< avm_number_t >::signaling_NaN());
}

void AsString::String_concat(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);

	StringOutputStream ss;
	ss << self->get();

	for (uint32_t i = 0; i < ca.args.size(); ++i)
		ss << ca.args[i].getStringSafe();

	ca.ret = ActionValue(ss.str());
}

void AsString::String_fromCharCode(CallArgs& ca)
{
	wchar_t charCode = wchar_t(ca.args[0].getNumberSafe());
	ca.ret = ActionValue(new String(charCode));
}

void AsString::String_indexOf(CallArgs& ca)
{
}

void AsString::String_lastIndexOf(CallArgs& ca)
{
}

void AsString::String_slice(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
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
	Ref< String > self = checked_type_cast< String* >(ca.self);
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

	Ref< Array > arr = new Array();
	for (std::vector< std::wstring >::const_iterator i = words.begin(); i != words.end(); ++i)
		arr->push(ActionValue(*i));

	ca.ret = ActionValue(arr);
}

void AsString::String_substr(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());
	uint32_t count = uint32_t(ca.args[1].getNumberSafe());

	ca.ret = ActionValue(st.substr(index, count));
}

void AsString::String_substring(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::wstring& st = self->get();

	uint32_t start = uint32_t(ca.args[0].getNumberSafe());
	uint32_t end = uint32_t(ca.args[1].getNumberSafe());

	ca.ret = ActionValue(st.substr(start, end - start));
}

void AsString::String_toLowerCase(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::wstring& st = self->get();
	ca.ret = ActionValue(new String(toLower(st)));
}

void AsString::String_toString(CallArgs& ca)
{
	ca.ret = ActionValue(ca.self);
}

void AsString::String_toUpperCase(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::wstring& st = self->get();
	ca.ret = ActionValue(new String(toUpper(st)));
}

void AsString::String_valueOf(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	ca.ret = ActionValue(self->get());
}

	}
}
