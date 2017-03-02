#include "Flash/FlashCast.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace flash
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

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args) T_OVERRIDE T_FINAL
	{
		m_delegate->call(0, 0);
		return ActionValue();
	}

	IRuntimeDelegate* getDelegate() const
	{
		return m_delegate;
	}

protected:
	virtual void dereference() T_OVERRIDE
	{
		m_delegate = 0;
		ActionFunction::dereference();
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionFunctionDelegate", ActionFunctionDelegate, ActionFunction)

Any castActionToAny(const ActionValue& value)
{
	switch (value.getType())
	{
	case flash::ActionValue::AvtBoolean:
		return Any::fromBoolean(value.getBoolean());
	case flash::ActionValue::AvtInteger:
		return Any::fromInteger(value.getInteger());
	case flash::ActionValue::AvtFloat:
		return Any::fromFloat(value.getFloat());
	case flash::ActionValue::AvtString:
		return Any::fromString(value.getString());
	case flash::ActionValue::AvtObject:
		{
			ActionObject* object = value.getObject();
			if (is_a< ActionFunctionDelegate* >(object))
				return Any::fromObject(static_cast< ActionFunctionDelegate* >(object)->getDelegate());
			else if (object->getRelay())
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
		return flash::ActionValue(value.getBooleanUnsafe());
	else if (value.isInteger())
		return flash::ActionValue(value.getIntegerUnsafe());
	else if (value.isFloat())
		return flash::ActionValue(value.getFloatUnsafe());
	else if (value.isString())
		return flash::ActionValue(value.getStringUnsafe());
	else if (value.isObject())
	{
		ITypedObject* object = value.getObjectUnsafe();
		if (is_a< IRuntimeDelegate* >(object))
			return flash::ActionValue(new ActionFunctionDelegate(static_cast< IRuntimeDelegate* >(object)));
		else if (is_a< flash::ActionObject* >(object))
			return flash::ActionValue(static_cast< flash::ActionObject* >(object));
		else if (is_a< flash::ActionObjectRelay* >(object))
			return flash::ActionValue(static_cast< flash::ActionObjectRelay* >(object)->getAsObject());
		else
			return flash::ActionValue();
	}
	else
		return flash::ActionValue();
}

	}
}
