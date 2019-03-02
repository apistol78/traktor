#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/Number.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Number", Number, ActionObjectRelay)

Number::Number(float value)
:	ActionObjectRelay("Number")
,	m_value(value)
{
}

	}
}
