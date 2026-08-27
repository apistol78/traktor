/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/DisplacementEntityComponentData.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity/DisplacementEntityComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DisplacementEntityComponentData", 0, DisplacementEntityComponentData, IEntityComponentData)

Ref< DisplacementEntityComponent > DisplacementEntityComponentData::createComponent() const
{
	return new DisplacementEntityComponent(this);
}

int32_t DisplacementEntityComponentData::getOrdinal() const
{
	return 0;
}

void DisplacementEntityComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void DisplacementEntityComponentData::serialize(ISerializer& s)
{
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"strength", m_strength, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"falloff", m_falloff, AttributeRange(0.0f));
	s >> Member< float >(L"press", m_press, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
}

}
