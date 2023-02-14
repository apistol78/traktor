/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/Editor/Material.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.Material", 0, Material, ISerializable)

void Material::setFriction(float friction)
{
	m_friction = friction;
}

float Material::getFriction() const
{
	return m_friction;
}

void Material::setRestitution(float restitution)
{
	m_restitution = restitution;
}

float Material::getRestitution() const
{
	return m_restitution;
}

void Material::serialize(ISerializer& s)
{
	s >> Member< float >(L"friction", m_friction, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"restitution", m_restitution, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
}

}
