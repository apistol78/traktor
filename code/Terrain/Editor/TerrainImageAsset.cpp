/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/TerrainImageAsset.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainImageAsset", 0, TerrainImageAsset, ISerializable)

void TerrainImageAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"instance", m_instance);
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< int32_t >(L"tileCount", m_tileCount);
	s >> Member< int32_t >(L"tileX", m_tileX);
	s >> Member< int32_t >(L"tileZ", m_tileZ);
	s >> Member< int32_t >(L"overviewSize", m_overviewSize);
	s >> Member< uint32_t >(L"stamp", m_stamp);
}

}
