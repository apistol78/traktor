/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class RenderGraph;

}

namespace traktor::world
{

class WorldEntityRenderers;
class WorldRenderView;

/*! Z pre-pass; renders depth only of all opaque geometry.
 *
 * The pass writes into the depth buffer shared with the following
 * geometry passes so they can rely on early-z rejection instead of
 * shading fragments which end up being occluded.
 */
class ZPrePass : public Object
{
	T_RTTI_CLASS;

public:
	explicit ZPrePass(
		const WorldRenderSettings& settings,
		WorldEntityRenderers* entityRenderers);

	void destroy();

	render::RGTargetSet setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		render::RenderGraph& renderGraph,
		render::RGTexture hiZTextureId,
		render::RGTargetSet outputTargetSetId) const;

private:
	WorldRenderSettings m_settings;
	Ref< WorldEntityRenderers > m_entityRenderers;
};

}
