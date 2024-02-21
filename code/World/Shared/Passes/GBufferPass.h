/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
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

class RenderGraph;

}

namespace traktor::world
{

class IrradianceGrid;
class WorldEntityRenderers;
class WorldRenderView;

/*!
 */
class GBufferPass : public Object
{
	T_RTTI_CLASS;

public:
	explicit GBufferPass(
		const WorldRenderSettings& settings,
		WorldEntityRenderers* entityRenderers
	);

	render::handle_t setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		const IrradianceGrid* irradianceGrid,
		render::handle_t gbufferWriteTechnique,
		render::RenderGraph& renderGraph,
		render::handle_t hiZTextureId,
		render::handle_t outputTargetSetId
	) const;

private:
	WorldRenderSettings m_settings;
	Ref< WorldEntityRenderers > m_entityRenderers;
};

}
