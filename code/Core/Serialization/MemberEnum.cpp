/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/ISerializer.h"

namespace traktor
{

MemberEnumBase::MemberEnumBase(const wchar_t* const name, const Key* keys)
:	MemberComplex(name, false)
,	m_keys(keys)
{
}

const MemberEnumBase::Key* MemberEnumBase::keys() const
{
	return m_keys;
}

void MemberEnumBase::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::SdRead)
	{
		std::wstring id;
		s >> Member< std::wstring >(getName(), id);
		s.ensure(set(id));
	}
	else	/* ISerializer::SdWrite */
	{
		const wchar_t* id = get();
		if (!s.ensure(id != 0))
			return;

		std::wstring ws(id);
		s >> Member< std::wstring >(getName(), ws);
	}
}

}
