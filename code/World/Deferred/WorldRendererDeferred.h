/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/StaticVector.h"
#include "World/Shared/WorldRendererShared.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ImageGraph;
class Shader;

}

namespace traktor::world
{

/*! World renderer, using deferred rendering method.
 * \ingroup World
 *
 * Advanced deferred rendering path.
 */
class T_DLLCLASS WorldRendererDeferred : public WorldRendererShared
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void setup(
		const World* world,
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId,
		const std::function< bool(const EntityState& state) >& filter
	) override final;

private:
	resource::Proxy< render::Shader > m_lightShader;

	void setupVisualPass(
		const WorldRenderView& worldRenderView,
		render::RenderGraph& renderGraph,
		render::handle_t visualWriteTargetSetId,
		render::handle_t gbufferTargetSetId,
		render::handle_t dbufferTargetSetId,
		render::handle_t hiZTextureId,
		render::handle_t ambientOcclusionTargetSetId,
		render::handle_t contactShadowsTargetSetId,
		render::handle_t reflectionsTargetSetId,
		render::handle_t shadowMapAtlasTargetSetId
	) const;
};

}
