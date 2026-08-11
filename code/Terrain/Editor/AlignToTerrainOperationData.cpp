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
#include "Core/Serialization/MemberEnum.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.AlignToTerrainOperationData", 2, AlignToTerrainOperationData, scene::ISceneOperationData)

void AlignToTerrainOperationData::serialize(ISerializer& s)
{
	const MemberEnum< GroundFit >::Key c_GroundFit_Keys[] = {
		{ L"Center", GroundFit::Center },
		{ L"Lowest", GroundFit::Lowest },
		{ L"Average", GroundFit::Average },
		{ L"Highest", GroundFit::Highest },
		{ 0 }
	};

	s >> MemberAlignedVector< std::wstring >(L"layers", m_layers);
	s >> Member< bool >(L"alignOrientation", m_alignOrientation);

	if (s.getVersion< AlignToTerrainOperationData >() >= 1)
		s >> Member< bool >(L"randomHeadingAngle", m_randomHeadingAngle);

	s >> Member< float >(L"upness", m_upness, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"offset", m_offset);

	if (s.getVersion< AlignToTerrainOperationData >() >= 2)
	{
		s >> MemberEnum< GroundFit >(L"groundFit", m_groundFit, c_GroundFit_Keys);
		s >> Member< float >(L"contactRatio", m_contactRatio, AttributeRange(0.0f, 1.0f));
	}
}

}
