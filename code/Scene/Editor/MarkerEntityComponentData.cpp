/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/MarkerEntityComponentData.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.MarkerEntityComponentData", 1, MarkerEntityComponentData, world::IEntityComponentData)

int32_t MarkerEntityComponentData::getOrdinal() const
{
	return 0;
}

void MarkerEntityComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void MarkerEntityComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::ITexture >(L"texture", m_texture);

	if (s.getVersion< MarkerEntityComponentData >() >= 1)
	{
		s >> Member< float >(L"offset", m_offset, AttributeUnit(UnitType::Metres));
		s >> Member< float >(L"size", m_size, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	}
}

}
