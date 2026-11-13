/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityStreamingHLODComponentData.h"

#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.EntityStreamingHLODComponentData", 0, EntityStreamingHLODComponentData, world::IEntityComponentData)

int32_t EntityStreamingHLODComponentData::getOrdinal() const
{
	return 0;
}

void EntityStreamingHLODComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void EntityStreamingHLODComponentData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"angles", m_angles, AttributeRange(1, 64));
	s >> Member< int32_t >(L"elevations", m_elevations, AttributeRange(1, 16));
	s >> Member< float >(L"elevationFrom", m_elevationFrom, AttributeRange(-HALF_PI, HALF_PI) | AttributeUnit(UnitType::Radians));
	s >> Member< float >(L"elevationTo", m_elevationTo, AttributeRange(-HALF_PI, HALF_PI) | AttributeUnit(UnitType::Radians));
	s >> Member< int32_t >(L"tileSize", m_tileSize, AttributeRange(16, 1024));
}

}
