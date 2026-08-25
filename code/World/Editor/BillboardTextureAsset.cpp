/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/BillboardTextureAsset.h"

#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "World/EntityData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.BillboardTextureAsset", 1, BillboardTextureAsset, ISerializable)

void BillboardTextureAsset::serialize(ISerializer& s)
{
	const MemberEnum< Content >::Key kContent[] = {
		{ L"Albedo", Content::Albedo },
		{ L"Normals", Content::Normals },
		{ 0 }
	};

	s >> Member< Guid >(L"entity", m_entity, AttributeType(type_of< EntityData >()));

	if (s.getVersion< BillboardTextureAsset >() >= 1)
		s >> MemberEnum< Content >(L"content", m_content, kContent);

	s >> Member< int32_t >(L"angles", m_angles, AttributeRange(1));
	s >> Member< int32_t >(L"elevations", m_elevations, AttributeRange(1));
	s >> Member< float >(L"elevationFrom", m_elevationFrom, AttributeRange(-HALF_PI, HALF_PI) | AttributeUnit(UnitType::Radians));
	s >> Member< float >(L"elevationTo", m_elevationTo, AttributeRange(-HALF_PI, HALF_PI) | AttributeUnit(UnitType::Radians));
	s >> Member< int32_t >(L"tileSize", m_tileSize, AttributeRange(1));
	s >> Member< float >(L"alphaThreshold", m_alphaThreshold, AttributeRange(0.0f, 1.0f));
	s >> Member< bool >(L"twoSided", m_twoSided);
	s >> Member< bool >(L"generateMips", m_generateMips);
	s >> Member< bool >(L"enableCompression", m_enableCompression);
}

}
