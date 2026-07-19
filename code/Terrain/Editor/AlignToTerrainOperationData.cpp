/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/AlignToTerrainOperationData.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.AlignToTerrainOperationData", 0, AlignToTerrainOperationData, scene::ISceneOperationData)

void AlignToTerrainOperationData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< std::wstring >(L"layers", m_layers);
	s >> Member< bool >(L"alignOrientation", m_alignOrientation);
	s >> Member< float >(L"upness", m_upness, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"offset", m_offset);
}

}
