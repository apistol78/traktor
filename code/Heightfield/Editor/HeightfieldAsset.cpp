/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/HeightfieldAsset.h"

#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldAsset", 10, HeightfieldAsset, ISerializable)

HeightfieldAsset::HeightfieldAsset(const Vector4& worldExtent)
	: m_worldExtent(worldExtent)
{
}

void HeightfieldAsset::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 4);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent, AttributeDirection());

	if (s.getVersion() >= 5 && s.getVersion() < 8)
		s >> ObsoleteMember< float >(L"vistaDistance", AttributeUnit(UnitType::Metres));

	if (s.getVersion() < 10)
	{
		if (s.getVersion() >= 6)
			s >> ObsoleteMember< bool >(L"erosionEnable");

		if (s.getVersion() >= 7)
			s >> ObsoleteMember< int32_t >(L"erodeIterations");
	}

	if (s.getVersion() >= 9)
		s >> Member< int32_t >(L"tileCount", m_tileCount);
}

}
