#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Action/Avm1/Classes/AsArray.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

struct ArrayPredicateSort
{
	ActionContext* context;
	ActionFunction* predicateFunction;
	ActionValueArray* predicateFunctionArgs;

	bool operator () (const ActionValue& avl, const ActionValue& avr) const
	{
		(*predicateFunctionArgs)[0] = avl;
		(*predicateFunctionArgs)[1] = avr;

		ActionValue resv = predicateFunction->call(context, 0, (*predicateFunctionArgs));
		int32_t res = int32_t(resv.getNumberSafe());

		return res < 0;
	}
};

struct ArrayDefaultSort
{
	bool operator () (const ActionValue& avl, const ActionValue& avr) const
	{
		// \fixme
		return false;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsArray", AsArray, ActionClass)

AsArray::AsArray()
:	ActionClass("Array")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("CASEINSENSITIVE", ActionValue(avm_number_t(0)));
	prototype->setMember("DESCENDING", ActionValue(avm_number_t(1)));
	prototype->setMember("NUMERIC", ActionValue(avm_number_t(2)));
	prototype->setMember("RETURNINDEXEDARRAY", ActionValue(avm_number_t(3)));
	prototype->setMember("UNIQUESORT", ActionValue(avm_number_t(4)));
	prototype->setMember("concat", ActionValue(createNativeFunction(this, &AsArray::Array_concat)));
	prototype->setMember("join", ActionValue(createNativeFunction(this, &AsArray::Array_join)));
	prototype->setMember("pop", ActionValue(createNativeFunction(this, &AsArray::Array_pop)));
	prototype->setMember("push", ActionValue(createNativeFunction(this, &AsArray::Array_push)));
	prototype->setMember("reverse", ActionValue(createNativeFunction(this, &AsArray::Array_reverse)));
	prototype->setMember("shift", ActionValue(createNativeFunction(this, &AsArray::Array_shift)));
	prototype->setMember("slice", ActionValue(createNativeFunction(this, &AsArray::Array_slice)));
	prototype->setMember("sort", ActionValue(createNativeFunction(this, &AsArray::Array_sort)));
	prototype->setMember("sortOn", ActionValue(createNativeFunction(this, &AsArray::Array_sortOn)));
	prototype->setMember("splice", ActionValue(createNativeFunction(this, &AsArray::Array_splice)));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &AsArray::Array_toString)));
	prototype->setMember("unshift", ActionValue(createNativeFunction(this, &AsArray::Array_unshift)));

	prototype->addProperty("length", createNativeFunction(this, &AsArray::Array_get_length), 0);

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsArray::construct(ActionContext* context, const ActionValueArray& args)
{
	Ref< Array > object = new Array();
	for (uint32_t i = 0; i < args.size(); ++i)
		object->push(args[i]);
	return ActionValue(object);
}

void AsArray::Array_concat(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);
	if (!ca.args.empty())
		ca.ret = ActionValue(arr->concat(ca.args));
	else
		ca.ret = ActionValue(arr->concat());
}

void AsArray::Array_join(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);
	if (ca.args.empty())
		ca.ret = ActionValue(arr->join(","));
	else
		ca.ret = ActionValue(arr->join(ca.args[0].getStringSafe()));
}

void AsArray::Array_pop(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);
	ca.ret = arr->pop();
}

void AsArray::Array_push(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);
	arr->push(ca.args[0]);
}

void AsArray::Array_reverse(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);
	arr->reverse();
	ca.ret = ActionValue(arr);
}

void AsArray::Array_shift(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);
	ca.ret = arr->shift();
}

void AsArray::Array_slice(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);
	
	int32_t startIndex = 0;
	int32_t endIndex = 16777215;
	
	if (ca.args.size() >= 1)
		startIndex = int32_t(ca.args[0].getNumberSafe());
	if (ca.args.size() >= 2)
		endIndex = int32_t(ca.args[1].getNumberSafe());

	ca.ret = ActionValue(arr->slice(startIndex, endIndex));
}

void AsArray::Array_sort(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);

	if (ca.args.size() >= 1)
	{
		ActionFunction* predicateFunction = ca.args[0].getObjectSafe< ActionFunction >();
		if (!predicateFunction)
			return;

		// Allocate argument array here as we don't want to flood the pool
		// with new array for each comparison.
		ActionValueArray predicateFunctionArgs(ca.context->getPool(), 2);

		ArrayPredicateSort aps;
		aps.context = ca.context;
		aps.predicateFunction = predicateFunction;
		aps.predicateFunctionArgs = &predicateFunctionArgs;

		arr->sort(aps);
	}
	else
		arr->sort(ArrayDefaultSort());
}

void AsArray::Array_sortOn(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsArray::Array_splice(CallArgs& ca)
{
	Array* arr = checked_type_cast< Array*, false >(ca.self);

	int32_t startIndex = int32_t(ca.args[0].getNumberSafe());
	uint32_t deleteCount = uint32_t(ca.args[1].getNumberSafe());

	Ref< Array > removed = arr->splice(startIndex, deleteCount, ca.args, 2);

	ca.ret = ActionValue(removed);
}

void AsArray::Array_toString(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	ca.ret = ActionValue(arr->join(","));
}

void AsArray::Array_unshift(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	ca.ret = ActionValue(avm_number_t(arr->unshift(ca.args)));
}

void AsArray::Array_get_length(CallArgs& ca)
{
	Ref< Array > arr = checked_type_cast< Array* >(ca.self);
	ca.ret = ActionValue(avm_number_t(arr->length()));
}

	}
}
