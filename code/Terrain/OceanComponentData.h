/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
	OceanComponentData();

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShaderWave() const { return m_shaderWave; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< Terrain >& getTerrain() const { return m_terrain; }

private:
	friend class OceanComponent;

	struct Wave
	{
		float direction = 0.0f;
		float amplitude = 0.0f;
		float frequency = 0.25f;
		float phase = 0.0f;
		float pinch = 3.0f;
		float rate = 4.0f;

		void serialize(ISerializer& s);
	};

	resource::Id< render::Shader > m_shaderWave;
	resource::Id< render::Shader > m_shader;
	resource::Id< Terrain > m_terrain;
	Color4f m_shallowTint;
	Color4f m_deepColor;
	float m_opacity;
	float m_elevation;
};

}
