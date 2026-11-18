/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::terrain
{

/*! Tile, or overview, of splat or color layer of terrain's heightfield; synthesized by terrain pipeline.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainImageAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	Guid m_heightfield;				//!< Heightfield asset instance containing layer.
	std::wstring m_name;			//!< Name of layer, "Splat" or "Color".
	int32_t m_tileX = -1;			//!< Tile, including +X/+Z seam; -1 for overview.
	int32_t m_tileZ = -1;
	int32_t m_overviewSize = 0;		//!< Size of overview of whole image.
	uint32_t m_stamp = 0;			//!< Stamp of tiles read; output depend on stamp instead of terrain instance.

	virtual void serialize(ISerializer& s) override final;
};

}
