/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "World/Entity/DecalEventData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DecalEventData", 1, DecalEventData, IEntityEventData)

void DecalEventData::serialize(ISerializer& s)
{
	if (s.getVersion< DecalEventData >() >= 1)
		s >> Member< Vector2 >(L"size", m_size, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	else
	{
		float size;
		s >> Member< float >(L"size", size, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
		m_size = Vector2(size, size);
	}
	s >> Member< float >(L"thickness", m_thickness, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"alpha", m_alpha, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"cullDistance", m_cullDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> resource::Member< render::Shader >(L"shader", m_shader);
}

}
