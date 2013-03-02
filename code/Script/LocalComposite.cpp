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

LocalComposite::LocalComposite(const std::wstring& name, const RefArray< Local >& values)
:	Local(name)
,	m_values(values)
{
}

const RefArray< Local >& LocalComposite::getValues() const
{
	return m_values;
}

bool LocalComposite::serialize(ISerializer& s)
{
	Local::serialize(s);
	s >> MemberRefArray< Local >(L"values", m_values);
	return true;
}

	}
}
