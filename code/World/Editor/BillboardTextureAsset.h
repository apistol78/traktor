/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Core/Ref.h"
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

class EntityData;

/*! Billboard atlas texture asset.
 * \ingroup World
 *
 * Entity rasterized into an atlas with one column per yaw angle and one row per elevation;
 * every tile frame the same square of given size around an offset.
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

	BillboardTextureAsset() = default;

	explicit BillboardTextureAsset(
		EntityData* entityData,
		Content content,
		int32_t angles,
		int32_t elevations,
		float elevationFrom,
		float elevationTo,
		int32_t tileSize,
		float size,
		const Vector4& offset);

	virtual void serialize(ISerializer& s) override final;

private:
	friend class BillboardTexturePipeline;

	/*! Entity which is rasterized into the atlas, in its own space. */
	Ref< EntityData > m_entityData;

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

	/*! World size of the square each view frame. */
	float m_size = 1.0f;

	/*! Centre of the framed square, in the entity's own space. */
	Vector4 m_offset = Vector4::zero();
};

}
