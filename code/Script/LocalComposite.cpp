#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Script/LocalComposite.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.LocalComposite", 0, LocalComposite, Local)

LocalComposite::LocalComposite()
{
}

LocalComposite::LocalComposite(const std::wstring& name, const std::wstring& value, const RefArray< Local >& values)
:	Local(name)
,	m_value(value)
,	m_values(values)
{
}

const std::wstring& LocalComposite::getValue() const
{
	return m_value;
}

const RefArray< Local >& LocalComposite::getValues() const
{
	return m_values;
}

void LocalComposite::serialize(ISerializer& s)
{
	Local::serialize(s);
	s >> Member< std::wstring >(L"value", m_value);
	s >> MemberRefArray< Local >(L"values", m_values);
}

	}
}
