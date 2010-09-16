#include "Flash/Action/Classes/Number.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Number", Number, ActionObject)

Number::Number(avm_number_t value)
:	ActionObject(L"Number")
,	m_value(value)
{
}

	}
}
