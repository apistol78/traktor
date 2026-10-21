/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityStreamingOperationData.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/Entity/PropertyEntityReference.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.EntityStreamingOperationData", 1, EntityStreamingOperationData, ISceneOperationData)

void EntityStreamingOperationData::serialize(ISerializer& s)
{
	s >> Member< float >(L"cellSize", m_cellSize, AttributeRange(1.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"loadDistance", m_loadDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"unloadDistance", m_unloadDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));

	if (s.getVersion< EntityStreamingOperationData >() >= 1)
		s >> MemberRefArray< world::PropertyEntityReference >(L"residentEntities", m_residentEntities);
}

}
