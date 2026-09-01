/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/GrassComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.GrassComponentData", 0, GrassComponentData, TerrainLayerComponentData)

void GrassComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"spreadDistance", m_spreadDistance);
	s >> MemberAlignedVector< Grass, MemberComposite< Grass > >(L"grass", m_grass);
}

void GrassComponentData::Grass::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"attribute", attribute);
	s >> Member< int32_t >(L"density", density);
	s >> Member< float >(L"scale", scale);
}

}
