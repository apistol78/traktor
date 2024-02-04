/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IWorldRenderer.h"

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

}

namespace traktor::world
{

class WorldEntityRenderers;

/*! World renderer implementation.
 * \ingroup World
 *
 * Simple world renderer, no support for
 * shadows, lights, postprocessing etc.
 *
 * Techniques used
 * "Simple" - Visual, final, color output.
 */
class T_DLLCLASS WorldRendererSimple : public IWorldRenderer
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
		EntityState filter
	) override final;

private:
	struct Gather
	{
		IEntityRenderer* entityRenderer;
		Object* renderable;
	};

	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< render::ITexture > m_depthTexture;
	AlignedVector< Gather > m_gathered;
};

}
