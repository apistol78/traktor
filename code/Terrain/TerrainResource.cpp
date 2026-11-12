/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Heightfield/Heightfield.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/TerrainResource.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainResource", 10, TerrainResource, ISerializable)

void TerrainResource::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 10);
	s >> Member< uint32_t >(L"detailSkip", m_detailSkip);
	s >> Member< uint32_t >(L"patchDim", m_patchDim);
	s >> resource::Member< render::Shader >(L"terrainShader", m_terrainShader);
	s >> resource::Member< render::Shader >(L"surfaceShader", m_surfaceShader);
	s >> Member< int32_t >(L"tileCount", m_tileCount);
	s >> Member< int32_t >(L"tileSize", m_tileSize);
	s >> MemberAlignedVector< Tile, MemberComposite< Tile > >(L"tiles", m_tiles);
	s >> MemberComposite< Tile >(L"overview", m_overview);
}

void TerrainResource::Tile::serialize(ISerializer& s)
{
	s >> resource::Member< hf::Heightfield >(L"heightfield", heightfield);
	s >> resource::Member< render::ITexture >(L"heightMap", heightMap);
	s >> resource::Member< render::ITexture >(L"normalMap", normalMap);
	s >> resource::Member< render::ITexture >(L"cutMap", cutMap);
	s >> resource::Member< render::ITexture >(L"splatMap", splatMap);
	s >> resource::Member< render::ITexture >(L"colorMap", colorMap);
}

}
