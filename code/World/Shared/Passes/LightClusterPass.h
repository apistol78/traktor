/*
 * TRAKTOR
 * Copyright (c) 2023-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class IRenderTargetSet;
class RenderGraph;

}

namespace traktor::world
{

class Entity;
class WorldEntityRenderers;
class WorldRenderView;

/*!
 */
class LightClusterPass : public Object
{
    T_RTTI_CLASS;

public:
	const static int32_t c_maxLightCount = 4096;

	//! Camera centered world grid used to find lights at ray hits.
	const static int32_t c_lightGridDim = 16;
	const static int32_t c_lightGridCellCount = c_lightGridDim * c_lightGridDim * c_lightGridDim;
	const static int32_t c_maxLightsPerGridCell = 32;
	static constexpr float c_lightGridExtent = 64.0f;

#pragma pack(1)
	struct LightIndexShaderData
	{
		int32_t lightIndex[4];
	};

	struct TileShaderData
	{
		int32_t lightOffsetAndCount[4];
	};
#pragma pack()

    explicit LightClusterPass(
        const WorldRenderSettings& settings
    );

	virtual ~LightClusterPass();

	bool create(render::IRenderSystem* renderSystem);

	void destroy();

	void setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView
	) const;

	render::Buffer* getLightIndexSBuffer() const { return m_lightIndexSBuffer; }

	render::Buffer* getTileSBuffer() const { return m_tileSBuffer; }

	render::Buffer* getLightGridSBuffer() const { return m_lightGridSBuffer; }

	render::Buffer* getLightGridIndexSBuffer() const { return m_lightGridIndexSBuffer; }

private:
    WorldRenderSettings m_settings;
	Ref< render::Buffer > m_lightIndexSBuffer;
	Ref< render::Buffer > m_tileSBuffer;
	Ref< render::Buffer > m_lightGridSBuffer;
	Ref< render::Buffer > m_lightGridIndexSBuffer;
};

}
