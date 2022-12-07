/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "SolutionBuilder/Dependency.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"Dependency", Dependency, ISerializable)

void Dependency::setInheritIncludePaths(bool inheritIncludePaths)
{
	m_inheritIncludePaths = inheritIncludePaths;
}

bool Dependency::getInheritIncludePaths() const
{
	return m_inheritIncludePaths;
}

void Dependency::setLink(Link link)
{
	m_link = link;
}

Dependency::Link Dependency::getLink() const
{
	return m_link;
}

void Dependency::serialize(ISerializer& s)
{
	if (s.getVersion() >= 3)
		s >> Member< bool >(L"inheritIncludePaths", m_inheritIncludePaths);

	if (s.getVersion() >= 2)
	{
		const MemberEnum< Link >::Key c_Link[] =
		{
			{ L"LnkNo", LnkNo },
			{ L"LnkYes", LnkYes },
			{ L"LnkForce", LnkForce }
		};
		s >> MemberEnum< Link >(L"link", m_link, c_Link);
	}
	else
	{
		bool linkWithProduct = true;
		s >> Member< bool >(L"linkWithProduct", linkWithProduct);
		m_link = linkWithProduct ? LnkYes : LnkNo;
	}
}

}
