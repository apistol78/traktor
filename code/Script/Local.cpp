#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/Local.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.Local", Local, ISerializable)

Local::Local()
{
}

Local::Local(const std::wstring& name)
:	m_name(name)
{
}

const std::wstring& Local::getName() const
{
	return m_name;
}

void Local::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
