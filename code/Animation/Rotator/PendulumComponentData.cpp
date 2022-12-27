/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/PendulumComponentData.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.PendulumComponentData", 0, PendulumComponentData, world::IEntityComponentData)

Ref< PendulumComponent > PendulumComponentData::createComponent() const
{
	return new PendulumComponent(m_axis, m_pivot, m_amplitude, m_rate);
}

void PendulumComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void PendulumComponentData::serialize(ISerializer& s)
{
	const MemberEnum< PendulumComponent::Axis >::Key c_Axis_keys[] =
	{
		{ L"X", PendulumComponent::Axis::X },
		{ L"Y", PendulumComponent::Axis::Y },
		{ L"Z", PendulumComponent::Axis::Z },
		{ 0 }
	};

	s >> MemberEnum< PendulumComponent::Axis >(L"axis", m_axis, c_Axis_keys);
    s >> Member< Vector4 >(L"pivot", m_pivot, AttributePoint());
	s >> Member< float >(L"amplitude", m_amplitude, AttributeUnit(UnitType::Degrees, false));
	s >> Member< float >(L"rate", m_rate, AttributeUnit(UnitType::Degrees, true));
}

}
