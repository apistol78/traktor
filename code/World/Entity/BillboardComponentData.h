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

class ITexture;
class Shader;

}

namespace traktor::world
{

/*! Billboard component data.
 * \ingroup World
 *
 * Quad textured from an atlas of the entity seen from a grid of angles;
 * grid must match the one the atlas was rendered with.
 */
class T_DLLCLASS BillboardComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	BillboardComponentData() = default;

	explicit BillboardComponentData(
		const resource::Id< render::Shader >& shader,
		const resource::Id< render::ITexture >& albedoTexture,
		const resource::Id< render::ITexture >& normalTexture,
		int32_t angles,
		int32_t elevations,
		float elevationFrom,
		float elevationTo,
		float size,
		const Vector4& offset);

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< render::ITexture >& getAlbedoTexture() const { return m_albedoTexture; }

	const resource::Id< render::ITexture >& getNormalTexture() const { return m_normalTexture; }

	int32_t getAngles() const { return m_angles; }

	int32_t getElevations() const { return m_elevations; }

	float getElevationFrom() const { return m_elevationFrom; }

	float getElevationTo() const { return m_elevationTo; }

	float getSize() const { return m_size; }

	const Vector4& getOffset() const { return m_offset; }

private:
	/*! Shader used to draw the quad; atlas textures are bound as "Billboard_Albedo" and "Billboard_Normals". */
	resource::Id< render::Shader > m_shader;

	/*! Atlas of surface color, with coverage in alpha. */
	resource::Id< render::ITexture > m_albedoTexture;

	/*! Atlas of surface normals in the space of each view. */
	resource::Id< render::ITexture > m_normalTexture;

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
};

}
