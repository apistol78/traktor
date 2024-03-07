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
#include "Resource/Proxy.h"
#include "World/WorldRenderSettings.h"

namespace traktor::render
{

class ImageGraph;
class IRenderSystem;
class RenderGraph;
class ScreenRenderer;

}

namespace traktor::world
{

struct WorldCreateDesc;

class WorldEntityRenderers;
class WorldRenderView;

/*!
 */
class PostProcessPass : public Object
{
	T_RTTI_CLASS;

public:
	explicit PostProcessPass(
		const WorldRenderSettings& settings,
		WorldEntityRenderers* entityRenderers
	);

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc);

	void setup(
		const WorldRenderView& worldRenderView,
		const GatherView& gatheredView,
		uint32_t frameCount,
		render::RenderGraph& renderGraph,
		render::handle_t gbufferTargetSetId,
		render::handle_t velocityTargetSetId,
		const DoubleBufferedTarget& visualTargetSetId,
		render::handle_t outputTargetSetId
	) const;

	bool needCameraJitter() const { return m_needCameraJitter; }

private:
	WorldRenderSettings m_settings;
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::ImageGraph > m_toneMap;
	resource::Proxy< render::ImageGraph > m_motionBlur;
	resource::Proxy< render::ImageGraph > m_antiAlias;
	resource::Proxy< render::ImageGraph > m_visual;
	resource::Proxy< render::ImageGraph > m_gammaCorrection;
	resource::Proxy< render::ITexture > m_colorGrading;
	float m_gamma = 2.2f;
	bool m_needCameraJitter = false;
};

}
