#include "Json/JsonArray.h"

namespace traktor
{
	namespace json
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonArray", JsonArray, JsonNode)

JsonArray::JsonArray()
{
}

void JsonArray::push(const Any& value)
{
	m_array.push_back(value);
}

	}
}
