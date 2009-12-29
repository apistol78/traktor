#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionArray.h"
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

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"CASEINSENSITIVE", ActionValue(0.0));
	prototype->setMember(L"DESCENDING", ActionValue(1.0));
	prototype->setMember(L"NUMERIC", ActionValue(2.0));
	prototype->setMember(L"RETURNINDEXEDARRAY", ActionValue(3.0));
	prototype->setMember(L"UNIQUESORT", ActionValue(4.0));
	prototype->setMember(L"concat", createNativeFunctionValue(this, &AsArray::Array_concat));
	prototype->setMember(L"join", createNativeFunctionValue(this, &AsArray::Array_join));
	prototype->setMember(L"pop", createNativeFunctionValue(this, &AsArray::Array_pop));
	prototype->setMember(L"push", createNativeFunctionValue(this, &AsArray::Array_push));
	prototype->setMember(L"reverse", createNativeFunctionValue(this, &AsArray::Array_reverse));
	prototype->setMember(L"shift", createNativeFunctionValue(this, &AsArray::Array_shift));
	prototype->setMember(L"slice", createNativeFunctionValue(this, &AsArray::Array_slice));
	prototype->setMember(L"sort", createNativeFunctionValue(this, &AsArray::Array_sort));
	prototype->setMember(L"sortOn", createNativeFunctionValue(this, &AsArray::Array_sortOn));
	prototype->setMember(L"splice", createNativeFunctionValue(this, &AsArray::Array_splice));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsArray::Array_toString));
	prototype->setMember(L"unshift", createNativeFunctionValue(this, &AsArray::Array_unshift));

	prototype->addProperty(L"length", createNativeFunction(this, &AsArray::Array_get_length), createNativeFunction(this, &AsArray::Array_set_length));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsArray::construct(ActionContext* context, const args_t& args)
{
	Ref< ActionArray > object = new ActionArray();
	for (args_t::const_iterator i = args.begin(); i != args.end(); ++i)
		object->push(*i);
	return ActionValue::fromObject(object);
}

void AsArray::Array_concat(CallArgs& ca)
{
	Ref< ActionArray > arr = checked_type_cast< ActionArray* >(ca.self);
	if (!ca.args.empty())
		ca.ret = ActionValue::fromObject(arr->concat(ca.args));
	else
		ca.ret = ActionValue::fromObject(arr->concat());
}

void AsArray::Array_join(CallArgs& ca)
{
	Ref< ActionArray > arr = checked_type_cast< ActionArray* >(ca.self);
	if (ca.args.empty())
		ca.ret = ActionValue(arr->join(L","));
	else
		ca.ret = ActionValue(arr->join(ca.args.front().getStringSafe()));
}

void AsArray::Array_pop(CallArgs& ca)
{
	Ref< ActionArray > arr = checked_type_cast< ActionArray* >(ca.self);
	ca.ret = arr->pop();
}

void AsArray::Array_push(CallArgs& ca)
{
	Ref< ActionArray > arr = checked_type_cast< ActionArray* >(ca.self);
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
	Ref< ActionArray > arr = checked_type_cast< ActionArray* >(ca.self);
	ca.ret = ActionValue(arr->join(L","));
}

void AsArray::Array_unshift(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_get_length(CallArgs& ca)
{
	Ref< ActionArray > arr = checked_type_cast< ActionArray* >(ca.self);
	ca.ret = ActionValue(double(arr->length()));
}

void AsArray::Array_set_length(CallArgs& ca)
{
	log::error << L"Array.length is read-only" << Endl;
}

	}
}
