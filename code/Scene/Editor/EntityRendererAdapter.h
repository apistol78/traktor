/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Ref.h"
#include "World/IEntityRenderer.h"

namespace traktor::scene
{

class EntityRendererCache;

class EntityRendererAdapter : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	explicit EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer, const std::function< bool(const EntityAdapter*) >& filter);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void setup(
		const world::WorldSetupContext& context,
		const world::WorldRenderView& worldRenderView,
		Object* renderable
	) override final;

	virtual void setup(
		const world::WorldSetupContext& context
	) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

private:
	Ref< EntityRendererCache > m_cache;
	Ref< world::IEntityRenderer > m_entityRenderer;
	std::function< bool(const EntityAdapter*) > m_filter;
};

}
