#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Action/Avm1/Classes/AsArray.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsArray", AsArray, ActionClass)

Ref< AsArray > AsArray::getInstance()
{
	static Ref< AsArray > instance = 0;
	if (!instance)
	{
		instance = new AsArray();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsArray::AsArray()
:	ActionClass(L"Array")
{
}

void AsArray::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"CASEINSENSITIVE", ActionValue(avm_number_t(0)));
	prototype->setMember(L"DESCENDING", ActionValue(avm_number_t(1)));
	prototype->setMember(L"NUMERIC", ActionValue(avm_number_t(2)));
	prototype->setMember(L"RETURNINDEXEDARRAY", ActionValue(avm_number_t(3)));
	prototype->setMember(L"UNIQUESORT", ActionValue(avm_number_t(4)));
	prototype->setMember(L"concat", ActionValue(createNativeFunction(this, &AsArray::Array_concat)));
	prototype->setMember(L"join", ActionValue(createNativeFunction(this, &AsArray::Array_join)));
	prototype->setMember(L"pop", ActionValue(createNativeFunction(this, &AsArray::Array_pop)));
	prototype->setMember(L"push", ActionValue(createNativeFunction(this, &AsArray::Array_push)));
	prototype->setMember(L"reverse", ActionValue(createNativeFunction(this, &AsArray::Array_reverse)));
	prototype->setMember(L"shift", ActionValue(createNativeFunction(this, &AsArray::Array_shift)));
	prototype->setMember(L"slice", ActionValue(createNativeFunction(this, &AsArray::Array_slice)));
	prototype->setMember(L"sort", ActionValue(createNativeFunction(this, &AsArray::Array_sort)));
	prototype->setMember(L"sortOn", ActionValue(createNativeFunction(this, &AsArray::Array_sortOn)));
	prototype->setMember(L"splice", ActionValue(createNativeFunction(this, &AsArray::Array_splice)));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsArray::Array_toString)));
	prototype->setMember(L"unshift", ActionValue(createNativeFunction(this, &AsArray::Array_unshift)));

	prototype->addProperty(L"length", createNativeFunction(this, &AsArray::Array_get_length), createNativeFunction(this, &AsArray::Array_set_length));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsArray::construct(ActionContext* context, const args_t& args)
{
	Ref< Array > object = new Array();
	for (args_t::const_iterator i = args.begin(); i != args.end(); ++i)
		object->push(*i);
	return ActionValue(object);
}

void AsArray::Array_concat(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	if (!ca.args.empty())
		ca.ret = ActionValue(arr->concat(ca.args));
	else
		ca.ret = ActionValue(arr->concat());
}

void AsArray::Array_join(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	if (ca.args.empty())
		ca.ret = ActionValue(arr->join(L","));
	else
		ca.ret = ActionValue(arr->join(ca.args.front().getStringSafe()));
}

void AsArray::Array_pop(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	ca.ret = arr->pop();
}

void AsArray::Array_push(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	arr->push(ca.args[0]);
}

void AsArray::Array_reverse(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_shift(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_slice(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_sort(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_sortOn(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_splice(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_toString(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	ca.ret = ActionValue(arr->join(L","));
}

void AsArray::Array_unshift(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_get_length(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	ca.ret = ActionValue(avm_number_t(arr->length()));
}

void AsArray::Array_set_length(CallArgs& ca)
{
	log::error << L"Array.length is read-only" << Endl;
}

	}
}
