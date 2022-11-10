/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Member.h"
#include "Terrain/RubbleComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.RubbleComponentData", 3, RubbleComponentData, TerrainLayerComponentData)

void RubbleComponentData::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 1);

	s >> Member< float >(L"spreadDistance", m_spreadDistance);
	s >> MemberAlignedVector< RubbleMesh, MemberComposite< RubbleMesh > >(L"rubble", m_rubble);
}

void RubbleComponentData::RubbleMesh::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::InstanceMesh >(L"mesh", mesh);

	if (s.getVersion< RubbleComponentData >() >= 2)
		s >> Member< uint8_t >(L"attribute", attribute);
	else
		s >> Member< uint8_t >(L"material", attribute);

	s >> Member< int32_t >(L"density", density, AttributeRange(0.0f));
	s >> Member< float >(L"randomScaleAmount", randomScaleAmount, AttributeRange(0.0f, 1.0f));

	if (s.getVersion< RubbleComponentData >() >= 3)
	{
		s >> Member< float >(L"randomTilt", randomTilt, AttributeRange(0.0f, HALF_PI) | AttributeUnit(UnitType::Radians));
		s >> Member< float >(L"upness", upness, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	}
}

}
