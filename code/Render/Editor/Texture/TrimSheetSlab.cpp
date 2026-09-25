/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetSlab.h"

#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Texture/TrimSheetRegion.h"

#include <algorithm>

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TrimSheetSlab", 0, TrimSheetSlab, ISerializable)

TrimSheetSlab::TrimSheetSlab(Orientation orientation, int32_t size)
:	m_orientation(orientation)
,	m_size(size)
{
}

int32_t TrimSheetSlab::getMargin() const
{
	int32_t margin = 0;
	for (auto region : m_regions)
		margin = std::max(margin, region->getMargin());
	return margin;
}

void TrimSheetSlab::insertRegion(int32_t index, TrimSheetRegion* region)
{
	index = clamp< int32_t >(index, 0, (int32_t)m_regions.size());
	m_regions.insert(m_regions.begin() + index, region);
}

void TrimSheetSlab::removeRegion(int32_t index)
{
	if (index >= 0 && index < (int32_t)m_regions.size())
		m_regions.erase(m_regions.begin() + index);
}

void TrimSheetSlab::serialize(ISerializer& s)
{
	const MemberEnum< Orientation >::Key c_Orientation_Keys[] = {
		{ L"Horizontal", Orientation::Horizontal },
		{ L"Vertical", Orientation::Vertical },
		{ 0 }
	};

	s >> MemberEnum< Orientation >(L"orientation", m_orientation, c_Orientation_Keys);
	s >> Member< int32_t >(L"size", m_size, AttributeRange(0));

	// Regions are edited through the trim sheet editor, not the property list.
	s >> MemberRefArray< TrimSheetRegion >(L"regions", m_regions, AttributePrivate());
}

}
