/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class TrimSheetRegion;

/*! Trim sheet slab.
 * \ingroup Render
 *
 * A horizontal slab spans the width of the remaining sheet area and
 * is taken from the top of it, a vertical slab spans the height and
 * is taken from the left. A slab is split, along its length, into
 * one or more regions.
 */
class T_DLLCLASS TrimSheetSlab : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum class Orientation
	{
		Horizontal,
		Vertical
	};

	TrimSheetSlab() = default;

	explicit TrimSheetSlab(Orientation orientation, int32_t size);

	void setOrientation(Orientation orientation) { m_orientation = orientation; }

	Orientation getOrientation() const { return m_orientation; }

	/*! Thickness, in pixels, excluding margins; 0 means all of the remaining sheet area. */
	void setSize(int32_t size) { m_size = size; }

	int32_t getSize() const { return m_size; }

	/*! Margin, on both sides, added to thickness; largest margin of all regions in slab. */
	int32_t getMargin() const;

	void insertRegion(int32_t index, TrimSheetRegion* region);

	void removeRegion(int32_t index);

	RefArray< TrimSheetRegion >& getRegions() { return m_regions; }

	const RefArray< TrimSheetRegion >& getRegions() const { return m_regions; }

	virtual void serialize(ISerializer& s) override final;

private:
	Orientation m_orientation = Orientation::Horizontal;
	int32_t m_size = 0;
	RefArray< TrimSheetRegion > m_regions;
};

}
