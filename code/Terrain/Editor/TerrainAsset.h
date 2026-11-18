/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class Heightfield;

}

namespace traktor::render
{

class Shader;

}

namespace traktor::terrain
{

/*! Terrain source asset.
 * \ingroup Terrain
 *
 * This contain source information about a terrain.
 * Heights, splat and color are layers of the referenced heightfield asset.
 */
class T_DLLCLASS TerrainAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	uint32_t getDetailSkip() const { return m_detailSkip; }

	uint32_t getPatchDim() const { return m_patchDim; }

	const resource::Id< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Id< render::Shader >& getSurfaceShader(int32_t index) const { return m_surfaceShaders[index]; }

private:
	uint32_t m_detailSkip = 1;
	uint32_t m_patchDim = 129;
	resource::Id< hf::Heightfield > m_heightfield;
	resource::Id< render::Shader > m_surfaceShaders[4];
};

}

