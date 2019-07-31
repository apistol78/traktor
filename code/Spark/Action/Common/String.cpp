#include "Spark/Action/ActionValue.h"
#include "Spark/Action/Common/String.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.String", String, ActionObjectRelay)

String::String()
:	ActionObjectRelay("String")
{
}

String::String(char ch)
:	ActionObjectRelay("String")
{
	m_str.resize(1, ch);
}

String::String(const std::string& str)
:	ActionObjectRelay("String")
,	m_str(str)
{
}

	}
}
