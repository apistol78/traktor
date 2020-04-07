#include "Core/Io/OutputStream.h"
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

bool JsonArray::write(OutputStream& os) const
{
	os << L"[" << Endl;
	os << IncreaseIndent;

	for (AlignedVector< Any >::const_iterator i = m_array.begin(); i != m_array.end(); ++i)
	{
		if (i != m_array.begin())
			os << L"," << Endl;

		switch (i->getType())
		{
		case Any::Type::Void:
			os << L"nil" << Endl;
			break;

		case Any::Type::Boolean:
			os << (i->getBooleanUnsafe() ? L"true" : L"false");
			break;

		case Any::Type::Int32:
			os << i->getInt32Unsafe();
			break;

		case Any::Type::Int64:
			os << i->getInt64Unsafe();
			break;

		case Any::Type::Float:
			os << i->getFloatUnsafe();
			break;

		case Any::Type::String:
			os << L"\"" << i->getWideString() << L"\"";
			break;

		case Any::Type::Object:
			{
				if (const JsonNode* node = dynamic_type_cast< const JsonNode* >(i->getObjectUnsafe()))
					node->write(os);
				else
					os << L"nil" << Endl;
			}
			break;
		}
	}
	if (!m_array.empty())
		os << Endl;

	os << DecreaseIndent;
	os << L"]" << Endl;
	return true;
}

	}
}
