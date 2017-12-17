/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "SolutionBuilder/Dependency.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"Dependency", Dependency, ISerializable)

Dependency::Dependency()
:	m_inheritIncludePaths(true)
,	m_link(LnkYes)
{
}

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
}
