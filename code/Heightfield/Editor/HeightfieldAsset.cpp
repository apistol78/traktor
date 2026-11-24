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
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeReadOnly.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Heightfield/Editor/ProceduralHeightfieldAsset.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldAsset", 12, HeightfieldAsset, ISerializable)

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
		s >> Member< int32_t >(L"tileCount", m_tileCount, AttributePrivate());

	// Earlier versions have a single base layer.
	if (s.getVersion() >= 12)
	{
		s >> Member< int32_t >(L"nextLayerId", m_nextLayerId, AttributePrivate());
		s >> MemberAlignedVector< Layer, MemberComposite< Layer > >(L"layers", m_layers, AttributeReadOnly());
		if (m_layers.empty())
			m_layers.push_back({ 0, L"Base", Guid() });
	}
	else if (s.getVersion() >= 11)
		s >> Member< Guid >(L"procedural", m_layers.front().procedural, AttributeType(type_of< ProceduralHeightfieldAsset >()));
}

void HeightfieldAsset::addLayer(const std::wstring& name)
{
	m_layers.push_back({ m_nextLayerId++, name, Guid() });
}

void HeightfieldAsset::Layer::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"id", id, AttributePrivate());
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"procedural", procedural, AttributeType(type_of< ProceduralHeightfieldAsset >()));
}

}
