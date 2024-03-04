/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/PathEntity/PathComponentData.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.PathComponentData", 1, PathComponentData, world::IEntityComponentData)

Ref< PathComponent > PathComponentData::createComponent() const
{
	return new PathComponent(m_path, m_timeMode);
}

int32_t PathComponentData::getOrdinal() const
{
	return -2000;
}

void PathComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void PathComponentData::serialize(ISerializer& s)
{
	s >> MemberComposite< TransformPath >(L"path", m_path);

	if (s.getVersion() >= 1)
	{
		const MemberEnum< PathComponent::TimeMode >::Key c_TimeMode_Keys[] =
		{
			{ L"Manual", PathComponent::TimeMode::Manual },
			{ L"Once", PathComponent::TimeMode::Once },
			{ L"Loop", PathComponent::TimeMode::Loop },
			{ L"PingPong", PathComponent::TimeMode::PingPong },
			{ 0 }
		};
		s >> MemberEnum< PathComponent::TimeMode >(L"timeMode", m_timeMode, c_TimeMode_Keys);
	}
	else
	{
		const MemberEnum< PathComponent::TimeMode >::Key c_TimeMode_Keys[] =
		{
			{ L"TmManual", PathComponent::TimeMode::Manual },
			{ L"TmOnce", PathComponent::TimeMode::Once },
			{ L"TmLoop", PathComponent::TimeMode::Loop },
			{ L"TmPingPong", PathComponent::TimeMode::PingPong },
			{ 0 }
		};
		s >> MemberEnum< PathComponent::TimeMode >(L"timeMode", m_timeMode, c_TimeMode_Keys);
	}

	if (s.getVersion() < 1)
		s >> ObsoleteMember< float >(L"timeOffset", AttributeUnit(UnitType::Seconds));
}

}
