#include "Json/JsonMember.h"

namespace traktor
{
	namespace json
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonMember", JsonMember, JsonNode)

JsonMember::JsonMember()
{
}

JsonMember::JsonMember(const std::wstring& name, const Any& value)
:	m_name(name)
,	m_value(value)
{
}

	}
}
