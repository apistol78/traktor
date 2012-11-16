#ifndef traktor_parade_FlashCast_H
#define traktor_parade_FlashCast_H

#include "Flash/Action/ActionValue.h"
#include "Script/CastAny.h"

namespace traktor
{
	namespace script
	{

template < >
struct CastAny < flash::ActionValue, false >
{
	static Any set(const flash::ActionValue& value)
	{
		switch (value.getType())
		{
		case flash::ActionValue::AvtBoolean:
			return Any(value.getBoolean());
		case flash::ActionValue::AvtNumber:
			return Any(float(value.getNumber()));
		case flash::ActionValue::AvtString:
			return Any(value.getWideString());
		case flash::ActionValue::AvtObject:
			return Any(value.getObject());
		default:
			return Any();
		}
	}

	static flash::ActionValue get(const Any& value)
	{
		if (value.isBoolean())
			return flash::ActionValue(value.getBoolean());
		else if (value.isInteger())
			return flash::ActionValue((flash::avm_number_t)value.getInteger());
		else if (value.isFloat())
			return flash::ActionValue(value.getFloat());
		else if (value.isString())
			return flash::ActionValue(value.getString());
		else if (value.isObject())
			return flash::ActionValue(dynamic_type_cast< flash::ActionObject* >(value.getObject()));
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
			return Any(value.getBoolean());
		case flash::ActionValue::AvtNumber:
			return Any(float(value.getNumber()));
		case flash::ActionValue::AvtString:
			return Any(value.getWideString());
		case flash::ActionValue::AvtObject:
			return Any(value.getObject());
		default:
			return Any();
		}
	}

	static flash::ActionValue get(const Any& value)
	{
		if (value.isBoolean())
			return flash::ActionValue(value.getBoolean());
		else if (value.isInteger())
			return flash::ActionValue((flash::avm_number_t)value.getInteger());
		else if (value.isFloat())
			return flash::ActionValue(value.getFloat());
		else if (value.isString())
			return flash::ActionValue(value.getString());
		else if (value.isObject())
			return flash::ActionValue(dynamic_type_cast< flash::ActionObject* >(value.getObject()));
		else
			return flash::ActionValue();
	}
};

	}
}

#endif	// traktor_parade_FlashCast_H
