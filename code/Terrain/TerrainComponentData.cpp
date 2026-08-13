/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/Member.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainComponentData", 1, TerrainComponentData, world::IEntityComponentData)

int32_t TerrainComponentData::getOrdinal() const
{
	return 0;
}

void TerrainComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void TerrainComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< Terrain >(L"terrain", m_terrain);
	if (s.getVersion< TerrainComponentData >() < 1)
	{
		s >> ObsoleteMember< float >(L"patchLodDistance");
		s >> ObsoleteMember< float >(L"patchLodBias");
		s >> ObsoleteMember< float >(L"patchLodExponent");
		s >> ObsoleteMember< float >(L"surfaceLodDistance");
		s >> ObsoleteMember< float >(L"surfaceLodBias");
		s >> ObsoleteMember< float >(L"surfaceLodExponent");
	}
}

}
