#include "Core/Io/OutputStream.h"
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

bool JsonMember::write(OutputStream& os) const
{
	os << L"\"" << m_name << L"\": ";
	switch (m_value.getType())
	{
	case Any::Type::Void:
		os << L"nil" << Endl;
		break;

	case Any::Type::Boolean:
		os << (m_value.getBooleanUnsafe() ? L"true" : L"false");
		break;

	case Any::Type::Int32:
		os << m_value.getInt32Unsafe();
		break;

	case Any::Type::Int64:
		os << m_value.getInt64Unsafe();
		break;

	case Any::Type::Float:
		os << m_value.getFloatUnsafe();
		break;

	case Any::Type::String:
		os << L"\"" << m_value.getWideString() << L"\"";
		break;

	case Any::Type::Object:
		{
			if (const JsonNode* node = dynamic_type_cast< const JsonNode* >(m_value.getObjectUnsafe()))
				node->write(os);
			else
				os << L"nil" << Endl;
		}
		break;
	}
	return true;
}

	}
}
