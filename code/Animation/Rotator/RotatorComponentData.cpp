/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/RotatorComponentData.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.RotatorComponentData", 0, RotatorComponentData, world::IEntityComponentData)

Ref< RotatorComponent > RotatorComponentData::createComponent() const
{
	return new RotatorComponent(m_axis, m_rate);
}

int32_t RotatorComponentData::getOrdinal() const
{
	return -1000;
}

void RotatorComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void RotatorComponentData::serialize(ISerializer& s)
{
	const MemberEnum< RotatorComponent::Axis >::Key c_Axis_keys[] =
	{
		{ L"X", RotatorComponent::Axis::X },
		{ L"Y", RotatorComponent::Axis::Y },
		{ L"Z", RotatorComponent::Axis::Z },
		{ 0 }
	};

	s >> MemberEnum< RotatorComponent::Axis >(L"axis", m_axis, c_Axis_keys);
	s >> Member< float >(L"rate", m_rate, AttributeUnit(UnitType::Degrees, true));
}

}
