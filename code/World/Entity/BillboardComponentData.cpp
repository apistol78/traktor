/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/BillboardComponentData.h"

#include "Core/Math/Const.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Shader.h"
#include "Resource/Member.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.BillboardComponentData", 0, BillboardComponentData, IEntityComponentData)

int32_t BillboardComponentData::getOrdinal() const
{
	return 0;
}

void BillboardComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void BillboardComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< int32_t >(L"angles", m_angles, AttributeRange(1));
	s >> Member< int32_t >(L"elevations", m_elevations, AttributeRange(1));
	s >> Member< float >(L"elevationFrom", m_elevationFrom, AttributeRange(-HALF_PI, HALF_PI) | AttributeUnit(UnitType::Radians));
	s >> Member< float >(L"elevationTo", m_elevationTo, AttributeRange(-HALF_PI, HALF_PI) | AttributeUnit(UnitType::Radians));
	s >> Member< float >(L"size", m_size, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< Vector4 >(L"offset", m_offset, AttributePoint());
	s >> Member< float >(L"startDistance", m_startDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"cullDistance", m_cullDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

}
