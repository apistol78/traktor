/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/DynamicBodyDesc.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.DynamicBodyDesc", 6, DynamicBodyDesc, BodyDesc)

DynamicBodyDesc::DynamicBodyDesc(ShapeDesc* shape)
:	BodyDesc(shape)
{
}

void DynamicBodyDesc::setMass(float mass)
{
	m_mass = mass;
}

float DynamicBodyDesc::getMass() const
{
	return m_mass;
}

void DynamicBodyDesc::setAutoDeactivate(bool autoDeactivate)
{
	m_autoDeactivate = autoDeactivate;
}

bool DynamicBodyDesc::getAutoDeactivate() const
{
	return m_autoDeactivate;
}

void DynamicBodyDesc::setActive(bool active)
{
	m_active = active;
}

bool DynamicBodyDesc::getActive() const
{
	return m_active;
}

void DynamicBodyDesc::setLinearDamping(float linearDamping)
{
	m_linearDamping = linearDamping;
}

float DynamicBodyDesc::getLinearDamping() const
{
	return m_linearDamping;
}

void DynamicBodyDesc::setAngularDamping(float angularDamping)
{
	m_angularDamping = angularDamping;
}

float DynamicBodyDesc::getAngularDamping() const
{
	return m_angularDamping;
}

void DynamicBodyDesc::setFriction(float friction)
{
	m_friction = friction;
}

float DynamicBodyDesc::getFriction() const
{
	return m_friction;
}

void DynamicBodyDesc::setRestitution(float restitution)
{
	m_restitution = restitution;
}

float DynamicBodyDesc::getRestitution() const
{
	return m_restitution;
}

void DynamicBodyDesc::setLinearThreshold(float linearThreshold)
{
	m_linearThreshold = linearThreshold;
}

float DynamicBodyDesc::getLinearThreshold() const
{
	return m_linearThreshold;
}

void DynamicBodyDesc::setAngularThreshold(float angularThreshold)
{
	m_angularThreshold = angularThreshold;
}

float DynamicBodyDesc::getAngularThreshold() const
{
	return m_angularThreshold;
}

void DynamicBodyDesc::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 4);

	BodyDesc::serialize(s);

	s >> Member< float >(L"mass", m_mass, AttributeRange(0.0f) | AttributeUnit(UnitType::Kilograms));
	s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
	s >> Member< bool >(L"active", m_active);
	s >> Member< float >(L"linearDamping", m_linearDamping, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"angularDamping", m_angularDamping, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"friction", m_friction, AttributeRange(0.0f));

	if (s.getVersion() >= 6)
		s >> Member< float >(L"restitution", m_restitution, AttributeRange(0.0f, 1.0f));

	if (s.getVersion() >= 5)
	{
		s >> Member< float >(L"linearThreshold", m_linearThreshold, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres, true));
		s >> Member< float >(L"angularThreshold", m_angularThreshold, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres, true));
	}
}

	}
}
