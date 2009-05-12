#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{

MemberEnumBase::MemberEnumBase(const std::wstring& name, const Key* keys)
:	MemberComplex(name, false)
,	m_keys(keys)
{
}

const MemberEnumBase::Key* MemberEnumBase::keys() const
{
	return m_keys;
}

bool MemberEnumBase::serialize(Serializer& s) const
{
	if (s.getDirection() == Serializer::SdRead)
	{
		std::wstring id;
		s >> Member< std::wstring >(getName(), id);
		return set(id);
	}
	else	/* Serializer::SdWrite */
	{
		if (!get())
			return false;
		std::wstring id = get();
		s >> Member< std::wstring >(getName(), id);
	}
	return true;
}

}
