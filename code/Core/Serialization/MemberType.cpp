#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberType.h"

namespace traktor
{

MemberType::MemberType(const wchar_t* const name, const TypeInfo*& type)
:	MemberComplex(name, false)
,	m_type(type)
{
}

void MemberType::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::Direction::Read)
	{
		std::wstring name;
		s >> Member< std::wstring >(getName(), name);
		if (!name.empty())
		{
			m_type = TypeInfo::find(name.c_str());
			s.ensure(m_type != nullptr);
		}
		else
			m_type = nullptr;
	}
	else	// Direction::Write
	{
		std::wstring name = m_type ? m_type->getName() : L"";
		s >> Member< std::wstring >(getName(), name);
	}
}

}
