#include "Core/Serialization/MemberType.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{

MemberType::MemberType(const std::wstring& name, const Type*& type)
:	MemberComplex(name, false)
,	m_type(type)
{
}

bool MemberType::serialize(Serializer& s) const
{
	if (s.getDirection() == Serializer::SdRead)
	{
		std::wstring name;
		s >> Member< std::wstring >(getName(), name);
		
		m_type = Type::find(name);
		return bool(m_type != 0);
	}
	else	// SdWrite
	{
		std::wstring name = m_type->getName();
		s >> Member< std::wstring >(getName(), name);
	}
	return true;
}

}
