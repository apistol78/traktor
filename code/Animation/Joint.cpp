/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Joint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Joint", 1, Joint, ISerializable)

Joint::Joint()
:	m_parent(-1)
,	m_transform(Transform::identity())
,	m_radius(0.1f)
{
}

void Joint::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< Joint >() >= 1);

	s >> Member< int32_t >(L"parent", m_parent);
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

}
