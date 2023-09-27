/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Net/Url.h"

namespace traktor::net
{

/*! URL serialization member.
 * \ingroup Net
 */
class MemberUrl : public MemberComplex
{
public:
	typedef Url value_type;

	MemberUrl(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		std::wstring url;
		if (s.getDirection() == ISerializer::Direction::Read)
		{
			s >> Member< std::wstring >(getName(), url);
			m_ref = Url(url);
		}
		else
		{
			url = m_ref.getString();
			s >> Member< std::wstring >(getName(), url);
		}
	}

private:
	value_type& m_ref;
};

}
