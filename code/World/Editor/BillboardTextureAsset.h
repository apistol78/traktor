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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Billboard atlas texture asset.
 * \ingroup World
 *
 * Rasterize a referenced entity from a number of angles into a single texture
 * atlas. Views are laid out as a grid; one column per yaw angle and one row per
 * elevation angle, with yaw increasing left to right starting at the entity's
 * front (-Z) and elevation increasing top to bottom.
 *
 * Every view share the same, rotation invariant, orthographic frustum so all
 * tiles map to a quad of the same world size regardless of angle; a billboard
 * component can therefore pick a tile from the view angle alone.
 *
 * One asset bake one map, \sa Content; pair two assets over the same entity when a
 * shader want to light the billboard from baked normals.
 */
class T_DLLCLASS BillboardTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! What a bake of the entity write into the atlas. */
	enum class Content
	{
		Albedo,  //!< Flat surface color, with coverage in alpha.
		Normals  //!< Surface normals in the space of each view, encoded as n * 0.5 + 0.5.
	};

	virtual void serialize(ISerializer& s) override final;

private:
	friend class BillboardTexturePipeline;

	/*! Entity which is rasterized into the atlas. */
	Guid m_entity;

	/*! Which map of the entity this asset bake.
	 *
	 * A shader referencing more than one map need one asset per map, since a texture is
	 * addressed by the guid of the asset producing it. Those assets must describe the
	 * same entity and the same grid, or their tiles will not line up.
	 */
	Content m_content = Content::Albedo;

	/*! Number of yaw angles, evenly distributed around the entity; columns in the atlas. */
	int32_t m_angles = 8;

	/*! Number of elevation angles; rows in the atlas. */
	int32_t m_elevations = 1;

	/*! Elevation of the first row, in radians, where zero is level with the entity. */
	float m_elevationFrom = 0.0f;

	/*! Elevation of the last row, in radians; ignored when there is only a single row. */
	float m_elevationTo = 0.0f;

	/*! Size, in pixels, of each view in the atlas. */
	int32_t m_tileSize = 256;

	/*! Discard fragments below this alpha, used to cut out alpha mapped foliage. */
	float m_alphaThreshold = 0.5f;

	/*! Rasterize back facing polygons as well; foliage is commonly single sided. */
	bool m_twoSided = true;

	bool m_generateMips = true;

	bool m_enableCompression = true;
};

}
