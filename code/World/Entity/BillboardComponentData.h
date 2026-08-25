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
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::world
{

/*! Billboard component data.
 * \ingroup World
 *
 * Stand in for a detailed entity at distance by drawing a single camera facing
 * quad textured from a pre-rendered atlas of the entity seen from a grid of
 * angles; \sa BillboardTextureAsset which produce such an atlas. The atlas itself
 * is bound by the shader, which is free to sample as many of the baked maps as
 * the lighting it want to do need.
 *
 * The grid described here must be the one the atlas was rendered with, since it
 * is what maps a view direction back onto a tile. The billboard pipeline log the
 * matching size and offset when it build the atlas.
 */
class T_DLLCLASS BillboardComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	int32_t getAngles() const { return m_angles; }

	int32_t getElevations() const { return m_elevations; }

	float getElevationFrom() const { return m_elevationFrom; }

	float getElevationTo() const { return m_elevationTo; }

	float getSize() const { return m_size; }

	const Vector4& getOffset() const { return m_offset; }

	float getStartDistance() const { return m_startDistance; }

	float getCullDistance() const { return m_cullDistance; }

private:
	/*! Shader used to draw the quad.
	 *
	 * The shader binds the atlas itself, so it is also what ties a billboard to a
	 * particular entity; it is expected to be double sided and to cut out on alpha.
	 */
	resource::Id< render::Shader > m_shader;

	/*! Number of yaw angles in the atlas. */
	int32_t m_angles = 8;

	/*! Number of elevation angles in the atlas. */
	int32_t m_elevations = 1;

	/*! Elevation of the first atlas row, in radians. */
	float m_elevationFrom = 0.0f;

	/*! Elevation of the last atlas row, in radians. */
	float m_elevationTo = 0.0f;

	/*! World size of the quad; the atlas frame the entity within a square of this side. */
	float m_size = 1.0f;

	/*! Centre of the quad in the entity's own space. */
	Vector4 m_offset = Vector4::zero();

	/*! Distance at which the billboard start to draw, letting the real entity cover closer range. */
	float m_startDistance = 0.0f;

	/*! Distance beyond which the billboard is not drawn at all. */
	float m_cullDistance = 200.0f;
};

}
