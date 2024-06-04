/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Core/Math/Vector2.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;
class ITexture;
class Shader;

}

namespace traktor::terrain
{

class Terrain;

/*! Ocean component data.
 * \ingroup Terrain
 */
class T_DLLCLASS OceanComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	struct Spectrum
	{
		float scale = 0.075f;
		float angle = -PI / 2.0f;
		float spreadBlend = 0.8f;
		float swell = 0.35f;
		float alpha = 0.2f;
		float peakOmega = 1.0f;
		float gamma = 0.5f;
		float shortWavesFade = 0.08f;

		void serialize(ISerializer& s);
	};

	OceanComponentData();

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShaderWave() const { return m_shaderWave; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< Terrain >& getTerrain() const { return m_terrain; }

private:
	friend class OceanComponent;

	resource::Id< render::Shader > m_shaderWave;
	resource::Id< render::Shader > m_shader;
	resource::Id< Terrain > m_terrain;
	Spectrum m_spectrum;
	Color4f m_shallowTint;
	Color4f m_deepColor;
	float m_opacity;
	float m_elevation;
};

}
