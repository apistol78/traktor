#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/LocalSimple.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.LocalSimple", 0, LocalSimple, Local)

LocalSimple::LocalSimple()
{
}

LocalSimple::LocalSimple(const std::wstring& name, const std::wstring& value)
:	Local(name)
,	m_value(value)
{
}

const std::wstring& LocalSimple::getValue() const
{
	return m_value;
}

void LocalSimple::serialize(ISerializer& s)
{
	Local::serialize(s);
	s >> Member< std::wstring >(L"value", m_value);
}

	}
}
