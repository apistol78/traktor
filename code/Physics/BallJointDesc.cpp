/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/BallJointDesc.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BallJointDesc", 0, BallJointDesc, JointDesc)

BallJointDesc::BallJointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
{
}

void BallJointDesc::setAnchor(const Vector4& anchor)
{
	m_anchor = anchor;
}

const Vector4& BallJointDesc::getAnchor() const
{
	return m_anchor;
}

void BallJointDesc::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"anchor", m_anchor, AttributePoint());
}

	}
}
