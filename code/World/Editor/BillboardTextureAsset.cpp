/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/BillboardTextureAsset.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "World/EntityData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.BillboardTextureAsset", 0, BillboardTextureAsset, ISerializable)

BillboardTextureAsset::BillboardTextureAsset(
	EntityData* entityData,
	Content content,
	int32_t angles,
	int32_t elevations,
	float elevationFrom,
	float elevationTo,
	int32_t tileSize,
	float size,
	const Vector4& offset)
	: m_entityData(entityData)
	, m_content(content)
	, m_angles(angles)
	, m_elevations(elevations)
	, m_elevationFrom(elevationFrom)
	, m_elevationTo(elevationTo)
	, m_tileSize(tileSize)
	, m_size(size)
	, m_offset(offset)
{
}

void BillboardTextureAsset::serialize(ISerializer& s)
{
	const MemberEnum< Content >::Key kContent[] = {
		{ L"Albedo", Content::Albedo },
		{ L"Normals", Content::Normals },
		{ 0 }
	};

	s >> MemberRef< EntityData >(L"entityData", m_entityData);
	s >> MemberEnum< Content >(L"content", m_content, kContent);
	s >> Member< int32_t >(L"angles", m_angles);
	s >> Member< int32_t >(L"elevations", m_elevations);
	s >> Member< float >(L"elevationFrom", m_elevationFrom);
	s >> Member< float >(L"elevationTo", m_elevationTo);
	s >> Member< int32_t >(L"tileSize", m_tileSize);
	s >> Member< float >(L"size", m_size);
	s >> Member< Vector4 >(L"offset", m_offset);
}

}
