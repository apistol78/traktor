/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/ISerializer.h"

namespace traktor
{

MemberEnumBase::MemberEnumBase(const wchar_t* const name)
:	MemberComplex(name, false)
{
}

void MemberEnumBase::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::Direction::Read)
	{
		std::wstring id;
		s >> Member< std::wstring >(getName(), id);
		s.ensure(set(id));
	}
	else	/* ISerializer::Direction::Write */
	{
		const wchar_t* id = get();
		if (!s.ensure(id != 0))
			return;

		std::wstring ws(id);
		s >> Member< std::wstring >(getName(), ws);
	}
}

}
