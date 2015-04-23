#ifndef traktor_flash_FlashCast_H
#define traktor_flash_FlashCast_H

#include "Core/Class/CastAny.h"
#include "Flash/Action/ActionValue.h"

namespace traktor
{

template < >
struct CastAny < flash::ActionValue, false >
{
	static Any set(const flash::ActionValue& value)
	{
		switch (value.getType())
		{
		case flash::ActionValue::AvtBoolean:
			return Any::fromBoolean(value.getBoolean());
		case flash::ActionValue::AvtNumber:
			return Any::fromFloat(float(value.getNumber()));
		case flash::ActionValue::AvtString:
			return Any::fromString(value.getString());
		case flash::ActionValue::AvtObject:
			return Any::fromObject(value.getObject());
		default:
			return Any();
		}
	}

	static flash::ActionValue get(const Any& value)
	{
		if (value.isBoolean())
			return flash::ActionValue(value.getBooleanUnsafe());
		else if (value.isInteger())
			return flash::ActionValue((flash::avm_number_t)value.getIntegerUnsafe());
		else if (value.isFloat())
			return flash::ActionValue(value.getFloatUnsafe());
		else if (value.isString())
			return flash::ActionValue(value.getStringUnsafe());
		else if (value.isObject())
			return flash::ActionValue(dynamic_type_cast< flash::ActionObject* >(value.getObjectUnsafe()));
		else
			return flash::ActionValue();
	}
};

template < >
struct CastAny < const flash::ActionValue&, false >
{
	static Any set(const flash::ActionValue& value)
	{
		switch (value.getType())
		{
		case flash::ActionValue::AvtBoolean:
			return Any::fromBoolean(value.getBoolean());
		case flash::ActionValue::AvtNumber:
			return Any::fromFloat(float(value.getNumber()));
		case flash::ActionValue::AvtString:
			return Any::fromString(value.getString());
		case flash::ActionValue::AvtObject:
			return Any::fromObject(value.getObject());
		default:
			return Any();
		}
	}

	static flash::ActionValue get(const Any& value)
	{
		if (value.isBoolean())
			return flash::ActionValue(value.getBooleanUnsafe());
		else if (value.isInteger())
			return flash::ActionValue((flash::avm_number_t)value.getIntegerUnsafe());
		else if (value.isFloat())
			return flash::ActionValue(value.getFloatUnsafe());
		else if (value.isString())
			return flash::ActionValue(value.getStringUnsafe());
		else if (value.isObject())
			return flash::ActionValue(dynamic_type_cast< flash::ActionObject* >(value.getObjectUnsafe()));
		else
			return flash::ActionValue();
	}
};

}

#endif	// traktor_flash_FlashCast_H
