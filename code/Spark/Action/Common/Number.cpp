#include "Spark/Action/ActionValue.h"
#include "Spark/Action/Common/Number.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Number", Number, ActionObjectRelay)

Number::Number(float value)
:	ActionObjectRelay("Number")
,	m_value(value)
{
}

	}
}
