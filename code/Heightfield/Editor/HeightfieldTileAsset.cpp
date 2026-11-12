/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/HeightfieldTileAsset.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Heightfield.h"
#include "Resource/Member.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldTileAsset", 0, HeightfieldTileAsset, ISerializable)

void HeightfieldTileAsset::serialize(ISerializer& s)
{
	s >> resource::Member< Heightfield >(L"heightfield", m_heightfield);
	s >> Member< int32_t >(L"tileX", m_tileX);
	s >> Member< int32_t >(L"tileZ", m_tileZ);
	s >> Member< int32_t >(L"overviewSize", m_overviewSize);
	s >> Member< uint32_t >(L"stamp", m_stamp);
}

}
