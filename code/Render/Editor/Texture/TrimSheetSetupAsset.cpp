/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"

#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Texture/TrimSheetSlab.h"

#include <algorithm>

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TrimSheetSetupAsset", 1, TrimSheetSetupAsset, ISerializable)

TrimSheetSetupAsset::TrimSheetSetupAsset()
{
	m_backgrounds[(int32_t)TrimSheetLayer::Albedo] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	m_backgrounds[(int32_t)TrimSheetLayer::Specular] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	m_backgrounds[(int32_t)TrimSheetLayer::Roughness] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	m_backgrounds[(int32_t)TrimSheetLayer::Normal] = Color4f(0.5f, 0.5f, 1.0f, 1.0f);
	m_backgrounds[(int32_t)TrimSheetLayer::Height] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
}

void TrimSheetSetupAsset::insertSlab(int32_t index, TrimSheetSlab* slab)
{
	index = clamp< int32_t >(index, 0, (int32_t)m_slabs.size());
	m_slabs.insert(m_slabs.begin() + index, slab);
}

void TrimSheetSetupAsset::removeSlab(int32_t index)
{
	if (index >= 0 && index < (int32_t)m_slabs.size())
		m_slabs.erase(m_slabs.begin() + index);
}

TrimSheetRegion* TrimSheetSetupAsset::getRegion(int32_t slab, int32_t region) const
{
	if (slab < 0 || slab >= (int32_t)m_slabs.size())
		return nullptr;

	const RefArray< TrimSheetRegion >& regions = m_slabs[slab]->getRegions();
	if (region < 0 || region >= (int32_t)regions.size())
		return nullptr;

	return regions[region];
}

void TrimSheetSetupAsset::calculateLayout(AlignedVector< TrimSheetRect >& outSlabs, AlignedVector< RegionLayout >& outRegions) const
{
	outSlabs.resize(0);
	outRegions.resize(0);

	TrimSheetRect remaining = { 0, 0, m_width, m_height };
	for (int32_t i = 0; i < (int32_t)m_slabs.size(); ++i)
	{
		const TrimSheetSlab* slab = m_slabs[i];
		const bool horizontal = (slab->getOrientation() == TrimSheetSlab::Orientation::Horizontal);

		// Margins are added to slab's thickness, on both sides.
		const int32_t margin = slab->getMargin();

		// Take slab from top, or left, of remaining sheet area.
		TrimSheetRect slabRect = remaining;
		if (horizontal)
		{
			slabRect.height = (slab->getSize() > 0) ? std::min(slab->getSize() + margin * 2, remaining.height) : remaining.height;
			remaining.y += slabRect.height;
			remaining.height -= slabRect.height;
		}
		else
		{
			slabRect.width = (slab->getSize() > 0) ? std::min(slab->getSize() + margin * 2, remaining.width) : remaining.width;
			remaining.x += slabRect.width;
			remaining.width -= slabRect.width;
		}
		outSlabs.push_back(slabRect);

		// Split slab along its length; fixed length regions get their length first
		// and whatever remains is shared evenly between the other regions.
		const RefArray< TrimSheetRegion >& regions = slab->getRegions();
		const int32_t length = horizontal ? slabRect.width : slabRect.height;

		int32_t fixedLength = 0;
		int32_t autoCount = 0;
		for (auto region : regions)
		{
			if (region->getSize() > 0)
				fixedLength += region->getSize();
			else
				++autoCount;
		}

		const int32_t autoLength = std::max(length - fixedLength, 0);
		int32_t autoIndex = 0;
		int32_t position = 0;

		for (int32_t j = 0; j < (int32_t)regions.size(); ++j)
		{
			int32_t regionLength = regions[j]->getSize();
			if (regionLength <= 0)
			{
				regionLength = autoLength / autoCount + ((autoIndex < autoLength % autoCount) ? 1 : 0);
				++autoIndex;
			}
			regionLength = clamp(regionLength, 0, length - position);

			RegionLayout& regionLayout = outRegions.push_back();
			regionLayout.slab = i;
			regionLayout.region = j;
			regionLayout.rect = slabRect;
			if (horizontal)
			{
				regionLayout.rect.x += position;
				regionLayout.rect.width = regionLength;
			}
			else
			{
				regionLayout.rect.y += position;
				regionLayout.rect.height = regionLength;
			}

			// Content of all regions in slab are aligned, inside of slab's margins.
			regionLayout.content = regionLayout.rect;
			if (horizontal)
			{
				regionLayout.content.y += margin;
				regionLayout.content.height = std::max(regionLayout.rect.height - margin * 2, 0);
			}
			else
			{
				regionLayout.content.x += margin;
				regionLayout.content.width = std::max(regionLayout.rect.width - margin * 2, 0);
			}

			position += regionLength;
		}
	}
}

void TrimSheetSetupAsset::serialize(ISerializer& s)
{
	const MemberEnum< NormalConvention >::Key c_NormalConvention_Keys[] = {
		{ L"OpenGL", NormalConvention::OpenGL },
		{ L"DirectX", NormalConvention::DirectX },
		{ 0 }
	};

	s >> Member< int32_t >(L"width", m_width, AttributeRange(1, 8192));
	s >> Member< int32_t >(L"height", m_height, AttributeRange(1, 8192));
	s >> MemberEnum< NormalConvention >(L"normalConvention", m_normalConvention, c_NormalConvention_Keys);
	s >> Member< Color4f >(L"backgroundAlbedo", m_backgrounds[(int32_t)TrimSheetLayer::Albedo]);
	s >> Member< Color4f >(L"backgroundSpecular", m_backgrounds[(int32_t)TrimSheetLayer::Specular]);
	if (s.getVersion< TrimSheetSetupAsset >() >= 1)
		s >> Member< Color4f >(L"backgroundRoughness", m_backgrounds[(int32_t)TrimSheetLayer::Roughness]);
	s >> Member< Color4f >(L"backgroundNormal", m_backgrounds[(int32_t)TrimSheetLayer::Normal]);
	s >> Member< Color4f >(L"backgroundHeight", m_backgrounds[(int32_t)TrimSheetLayer::Height]);

	// Slabs are edited through the trim sheet editor, not the property list.
	s >> MemberRefArray< TrimSheetSlab >(L"slabs", m_slabs, AttributePrivate());
}

}
