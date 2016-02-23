#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/Value.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.Value", 0, Value, IValue)

Value::Value()
{
}

Value::Value(const std::wstring& literal)
:	m_literal(literal)
{
}

void Value::setLiteral(const std::wstring& literal)
{
	m_literal = literal;
}

const std::wstring& Value::getLiteral() const
{
	return m_literal;
}

void Value::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"literal", m_literal);
}

	}
}
