/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/HeightfieldAsset.h"

#include "Core/Guid.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Heightfield/Editor/EditLayer.h"

namespace traktor::hf
{
namespace
{

//! Layer as serialized before layers were objects; procedural heightfield assets are no longer supported.
struct LegacyLayer
{
	int32_t id = 0;
	std::wstring name;

	void serialize(ISerializer& s)
	{
		s >> Member< int32_t >(L"id", id);
		s >> Member< std::wstring >(L"name", name);
		s >> ObsoleteMember< Guid >(L"procedural");
	}
};

Ref< EditLayer > createBaseLayer()
{
	Ref< EditLayer > layer = new EditLayer();
	layer->setName(L"Base");
	return layer;
}

}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldAsset", 13, HeightfieldAsset, ISerializable)

HeightfieldAsset::HeightfieldAsset()
{
	m_layers.push_back(createBaseLayer());
}

HeightfieldAsset::HeightfieldAsset(const Vector4& worldExtent)
	: HeightfieldAsset()
{
	m_worldExtent = worldExtent;
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

	// Earlier versions have a single base layer; layers of version 12 are converted into edit layers.
	if (s.getVersion() >= 13)
	{
		s >> Member< int32_t >(L"nextLayerId", m_nextLayerId, AttributePrivate());
		s >> MemberRefArray< HeightfieldAssetLayer >(L"layers", m_layers, AttributePrivate());
	}
	else if (s.getVersion() >= 12)
	{
		AlignedVector< LegacyLayer > layers;
		s >> Member< int32_t >(L"nextLayerId", m_nextLayerId, AttributePrivate());
		s >> MemberAlignedVector< LegacyLayer, MemberComposite< LegacyLayer > >(L"layers", layers);

		m_layers.resize(0);
		for (const auto& layer : layers)
		{
			Ref< EditLayer > editLayer = new EditLayer();
			editLayer->m_id = layer.id;
			editLayer->setName(layer.name);
			m_layers.push_back(editLayer);
		}
	}
	else if (s.getVersion() >= 11)
		s >> ObsoleteMember< Guid >(L"procedural");

	if (m_layers.empty())
		m_layers.push_back(createBaseLayer());
}

void HeightfieldAsset::addLayer(HeightfieldAssetLayer* layer)
{
	layer->m_id = m_nextLayerId++;
	m_layers.push_back(layer);
}

}
