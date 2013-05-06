#include "Core/Io/StringOutputStream.h"
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
	ActionFunction* predicateFunction;
	ActionValueArray* predicateFunctionArgs;

	bool operator () (const ActionValue& avl, const ActionValue& avr) const
	{
		(*predicateFunctionArgs)[0] = avl;
		(*predicateFunctionArgs)[1] = avr;

		ActionValue resv = predicateFunction->call(0, 0, (*predicateFunctionArgs));
		int32_t res = int32_t(resv.getNumber());

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

AsArray::AsArray(ActionContext* context)
:	ActionClass(context, "Array")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("CASEINSENSITIVE", ActionValue(avm_number_t(0)));
	prototype->setMember("DESCENDING", ActionValue(avm_number_t(1)));
	prototype->setMember("NUMERIC", ActionValue(avm_number_t(2)));
	prototype->setMember("RETURNINDEXEDARRAY", ActionValue(avm_number_t(3)));
	prototype->setMember("UNIQUESORT", ActionValue(avm_number_t(4)));
	prototype->setMember("concat", ActionValue(createNativeFunction(context, this, &AsArray::Array_concat)));
	prototype->setMember("join", ActionValue(createNativeFunction(context, this, &AsArray::Array_join)));
	prototype->setMember("pop", ActionValue(createNativeFunction(context, &Array::pop)));
	prototype->setMember("push", ActionValue(createNativeFunction(context, &Array::push)));
	prototype->setMember("reverse", ActionValue(createNativeFunction(context, &Array::reverse)));
	prototype->setMember("shift", ActionValue(createNativeFunction(context, &Array::shift)));
	prototype->setMember("slice", ActionValue(createNativeFunction(context, this, &AsArray::Array_slice)));
	prototype->setMember("sort", ActionValue(createNativeFunction(context, this, &AsArray::Array_sort)));
	prototype->setMember("sortOn", ActionValue(createNativeFunction(context, this, &AsArray::Array_sortOn)));
	prototype->setMember("splice", ActionValue(createNativeFunction(context, this, &AsArray::Array_splice)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsArray::Array_toString)));
	prototype->setMember("unshift", ActionValue(createNativeFunction(context, this, &AsArray::Array_unshift)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsArray::initialize(ActionObject* self)
{
	ActionContext* context = getContext();

	self->addProperty("length", createNativeFunction(context, this, &AsArray::Array_get_length), createNativeFunction(context, this, &AsArray::Array_set_length));
}

void AsArray::construct(ActionObject* self, const ActionValueArray& args)
{
	self->setRelay(new Array(args));
}

ActionValue AsArray::xplicit(const ActionValueArray& args)
{
	if (args.size() > 0)
	{
		if (args[0].isObject())
			return args[0];
	}
	return ActionValue();
}

void AsArray::Array_concat(CallArgs& ca)
{
	Array* arr = ca.self->getRelay< Array >();
	if (arr)
	{
		if (!ca.args.empty())
			ca.ret = ActionValue(arr->concat(ca.args)->getAsObject(ca.context));
		else
			ca.ret = ActionValue(arr->concat()->getAsObject(ca.context));
	}
}

void AsArray::Array_join(CallArgs& ca)
{
	Array* arr = ca.self->getRelay< Array >();
	if (arr)
	{
		if (ca.args.empty())
			ca.ret = ActionValue(arr->join(","));
		else
			ca.ret = ActionValue(arr->join(ca.args[0].getString()));
	}
}

void AsArray::Array_slice(CallArgs& ca)
{
	Array* arr = ca.self->getRelay< Array >();
	if (arr)
	{
		int32_t startIndex = 0;
		int32_t endIndex = 16777215;
		
		if (ca.args.size() >= 1)
			startIndex = int32_t(ca.args[0].getNumber());
		if (ca.args.size() >= 2)
			endIndex = int32_t(ca.args[1].getNumber());

		ca.ret = ActionValue(arr->slice(startIndex, endIndex)->getAsObject(ca.context));
	}
}

void AsArray::Array_sort(CallArgs& ca)
{
	Array* arr = ca.self->getRelay< Array >();
	if (arr)
	{
		if (ca.args.size() >= 1)
		{
			ActionFunction* predicateFunction = ca.args[0].getObject< ActionFunction >();
			if (!predicateFunction)
				return;

			// Allocate argument array here as we don't want to flood the pool
			// with new array for each comparison.
			ActionValueArray predicateFunctionArgs(ca.context->getPool(), 2);

			ArrayPredicateSort aps;
			aps.predicateFunction = predicateFunction;
			aps.predicateFunctionArgs = &predicateFunctionArgs;

			arr->sort(aps);
		}
		else
			arr->sort(ArrayDefaultSort());
	}
}

void AsArray::Array_sortOn(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Array::sortOn not implemented" << Endl;
	)
}

void AsArray::Array_splice(CallArgs& ca)
{
	Array* arr = ca.self->getRelay< Array >();
	if (arr)
	{
		int32_t startIndex = int32_t(ca.args[0].getNumber());
		uint32_t deleteCount = uint32_t(ca.args[1].getNumber());

		Ref< Array > removed = arr->splice(startIndex, deleteCount, ca.args, 2);
		ca.ret = ActionValue(removed->getAsObject(ca.context));
	}
}

ActionValue AsArray::Array_toString(const Array* self) const
{
	StringOutputStream ss;

	const AlignedVector< ActionValue >& values = self->getValues();
	for (AlignedVector< ActionValue >::const_iterator i = values.begin(); i != values.end(); ++i)
	{
		if (i != values.begin())
			ss << L",";
		ss << i->getWideString();
	}

	return ActionValue(ss.str());
}

void AsArray::Array_unshift(CallArgs& ca)
{
	Ref< Array > arr = ca.self->getRelay< Array >();
	ca.ret = ActionValue(avm_number_t(arr->unshift(ca.args)));
}

uint32_t AsArray::Array_get_length(const Array* self) const
{
	return self->length();
}

void AsArray::Array_set_length(Array* self, uint32_t length) const
{
	if (length == 0)
		self->removeAll();
	else
	{
		while (self->length() > length)
			self->pop();
		while (self->length() < length)
			self->push(ActionValue());
	}
}

	}
}
