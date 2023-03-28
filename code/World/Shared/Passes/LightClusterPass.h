/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
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
	const static int32_t c_maxLightCount = 16;

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

	bool create(render::IRenderSystem* renderSystem);

	void destroy();

	void setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView
	) const;

	render::Buffer* getLightIndexSBuffer() const { return m_lightIndexSBuffer; }

	render::Buffer* getTileSBuffer() const { return m_tileSBuffer; }

private:
    WorldRenderSettings m_settings;
	Ref< render::Buffer > m_lightIndexSBuffer;
	Ref< render::Buffer > m_tileSBuffer;
};

}
