/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Editor/HeightfieldAsset.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldAsset", 7, HeightfieldAsset, ISerializable)

HeightfieldAsset::HeightfieldAsset()
:	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
,	m_vistaDistance(0.0f)
,	m_erosionEnable(false)
,	m_erodeIterations(100000)
{
}

HeightfieldAsset::HeightfieldAsset(const Vector4& worldExtent)
:	m_worldExtent(worldExtent)
{
}

void HeightfieldAsset::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 4);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent, AttributeDirection());

	if (s.getVersion() >= 5)
		s >> Member< float >(L"vistaDistance", m_vistaDistance);

	if (s.getVersion() >= 6)
		s >> Member< bool >(L"erosionEnable", m_erosionEnable);

	if (s.getVersion() >= 7)
		s >> Member< int32_t >(L"erodeIterations", m_erodeIterations);
}

	}
}
