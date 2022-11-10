/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeHex.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/CollisionSpecification.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CollisionSpecification", 0, CollisionSpecification, ISerializable)

CollisionSpecification::CollisionSpecification()
:	m_bitMask(0)
{
}

CollisionSpecification::CollisionSpecification(uint32_t bitMask)
:	m_bitMask(bitMask)
{
}

uint32_t CollisionSpecification::getBitMask() const
{
	return m_bitMask;
}

void CollisionSpecification::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"bitMask", m_bitMask, AttributeHex());
}

	}
}
