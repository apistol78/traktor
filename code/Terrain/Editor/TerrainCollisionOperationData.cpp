/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/TerrainCollisionOperationData.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainCollisionOperationData", 0, TerrainCollisionOperationData, scene::ISceneOperationData)

void TerrainCollisionOperationData::serialize(ISerializer& s)
{
	s >> Member< float >(L"loadDistance", m_loadDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

}
