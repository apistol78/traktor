/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/AxisJointDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.AxisJointDesc", 0, AxisJointDesc, JointDesc)

AxisJointDesc::AxisJointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_axis(1.0f, 0.0f, 0.0f, 0.0f)
{
}

void AxisJointDesc::setAnchor(const Vector4& anchor)
{
	m_anchor = anchor;
}

const Vector4& AxisJointDesc::getAnchor() const
{
	return m_anchor;
}

void AxisJointDesc::setAxis(const Vector4& axis)
{
	m_axis = axis;
}

const Vector4& AxisJointDesc::getAxis() const
{
	return m_axis;
}

void AxisJointDesc::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"anchor", m_anchor, AttributePoint());
	s >> Member< Vector4 >(L"axis", m_axis, AttributeDirection());
}

	}
}
