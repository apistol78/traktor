#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/ISerializer.h"

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

bool MemberEnumBase::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::SdRead)
	{
		std::wstring id;
		s >> Member< std::wstring >(getName(), id);
		return set(id);
	}
	else	/* ISerializer::SdWrite */
	{
		if (!get())
			return false;
		std::wstring id = get();
		s >> Member< std::wstring >(getName(), id);
	}
	return true;
}

}
