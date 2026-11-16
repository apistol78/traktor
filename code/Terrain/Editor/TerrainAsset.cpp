/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/TerrainAsset.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Heightfield/Heightfield.h"
#include "Render/Shader.h"
#include "Resource/Member.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainAsset", 7, TerrainAsset, ISerializable)

void TerrainAsset::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 6);

	s >> Member< uint32_t >(L"detailSkip", m_detailSkip);
	s >> Member< uint32_t >(L"patchDim", m_patchDim);
	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);

	if (s.getVersion< TerrainAsset >() >= 7)
	{
		s >> MemberStaticArray< resource::Id< render::Shader >, sizeof_array(m_surfaceShaders), resource::Member< render::Shader > >(L"surfaceShaders", m_surfaceShaders);
	}
	else
	{
		resource::Id< render::Shader > surfaceShader;
		s >> resource::Member< render::Shader >(L"surfaceShader", surfaceShader);
		m_surfaceShaders[0] = surfaceShader;
	}
}

}
