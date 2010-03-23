#include "Flash/Action/Classes/Number.h"
#include "Flash/Action/Avm1/Classes/AsNumber.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Number", Number, ActionObject)

Number::Number(avm_number_t value)
:	ActionObject(AsNumber::getInstance())
,	m_value(value)
{
}

	}
}
