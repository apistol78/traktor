#include "Spark/Action/Common/Boolean.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Boolean", Boolean, ActionObjectRelay)

Boolean::Boolean(bool value)
:	ActionObjectRelay("Boolean")
,	m_value(value)
{
}

	}
}
