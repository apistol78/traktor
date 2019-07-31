#include "Spark/Cast.h"
#include "Spark/Action/ActionFunction.h"
#include "Spark/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace spark
	{

class ActionFunctionDelegate : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionFunctionDelegate(IRuntimeDelegate* delegate)
	:	ActionFunction(0, "<delegate>")
	,	m_delegate(delegate)
	{
	}

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args) override final
	{
		Any argv[16];
		for (uint32_t i = 0; i < args.size(); ++i)
			argv[i] = castActionToAny(args[i]);
		Any retv = m_delegate->call(args.size(), argv);
		return castAnyToAction(retv);
	}

	IRuntimeDelegate* getDelegate() const
	{
		return m_delegate;
	}

protected:
	virtual void dereference() override
	{
		m_delegate = 0;
		ActionFunction::dereference();
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionFunctionDelegate", ActionFunctionDelegate, ActionFunction)

Any castActionToAny(const ActionValue& value)
{
	switch (value.getType())
	{
	case spark::ActionValue::AvtBoolean:
		return Any::fromBoolean(value.getBoolean());
	case spark::ActionValue::AvtInteger:
		return Any::fromInt32(value.getInteger());
	case spark::ActionValue::AvtFloat:
		return Any::fromFloat(value.getFloat());
	case spark::ActionValue::AvtString:
		return Any::fromString(value.getString());
	case spark::ActionValue::AvtObject:
		{
			ActionObject* object = value.getObject();
			if (is_a< ActionFunctionDelegate* >(object))
				return Any::fromObject(static_cast< ActionFunctionDelegate* >(object)->getDelegate());
			else if (object && object->getRelay())
				return Any::fromObject(object->getRelay());
			else
				return Any::fromObject(object);
		}
	default:
		return Any();
	}
}

ActionValue castAnyToAction(const Any& value)
{
	if (value.isBoolean())
		return spark::ActionValue(value.getBooleanUnsafe());
	else if (value.isInt32())
		return spark::ActionValue(value.getInt32Unsafe());
	else if (value.isInt64())
		return spark::ActionValue(int32_t(value.getInt64Unsafe()));
	else if (value.isFloat())
		return spark::ActionValue(value.getFloatUnsafe());
	else if (value.isString())
		return spark::ActionValue(value.getStringUnsafe());
	else if (value.isObject())
	{
		ITypedObject* object = value.getObjectUnsafe();
		if (is_a< IRuntimeDelegate* >(object))
			return spark::ActionValue(new ActionFunctionDelegate(static_cast< IRuntimeDelegate* >(object)));
		else if (is_a< spark::ActionObject* >(object))
			return spark::ActionValue(static_cast< spark::ActionObject* >(object));
		else if (is_a< spark::ActionObjectRelay* >(object))
			return spark::ActionValue(static_cast< spark::ActionObjectRelay* >(object)->getAsObject());
		else
			return spark::ActionValue();
	}
	else
		return spark::ActionValue();
}

	}
}
