/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/StaticBodyDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.StaticBodyDesc", 5, StaticBodyDesc, BodyDesc)

StaticBodyDesc::StaticBodyDesc(ShapeDesc* shape)
:	BodyDesc(shape)
{
}

void StaticBodyDesc::setFriction(float friction)
{
	m_friction = friction;
}

float StaticBodyDesc::getFriction() const
{
	return m_friction;
}

void StaticBodyDesc::setRestitution(float restitution)
{
	m_restitution = restitution;
}

float StaticBodyDesc::getRestitution() const
{
	return m_restitution;
}

void StaticBodyDesc::setKinematic(bool kinematic)
{
	m_kinematic = kinematic;
}

bool StaticBodyDesc::isKinematic() const
{
	return m_kinematic;
}

void StaticBodyDesc::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 4);

	BodyDesc::serialize(s);

	s >> Member< float >(L"friction", m_friction, AttributeRange(0.0f, 1.0f));

	if (s.getVersion() >= 5)
		s >> Member< float >(L"restitution", m_restitution, AttributeRange(0.0f, 1.0f));

	s >> Member< bool >(L"kinematic", m_kinematic);
}

	}
}
