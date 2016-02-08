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
		case Any::AtVoid:
			os << L"nil" << Endl;
			break;

		case Any::AtBoolean:
			os << (i->getBooleanUnsafe() ? L"true" : L"false");
			break;

		case Any::AtInteger:
			os << i->getIntegerUnsafe();
			break;

		case Any::AtFloat:
			os << i->getFloatUnsafe();
			break;

		case Any::AtString:
			os << L"\"" << i->getWideString() << L"\"";
			break;

		case Any::AtObject:
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
