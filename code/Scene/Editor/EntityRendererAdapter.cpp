/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityRendererAdapter", EntityRendererAdapter, world::IEntityRenderer)

EntityRendererAdapter::EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer, const std::function< bool(const EntityAdapter*) >& filter)
:	m_cache(cache)
,	m_entityRenderer(entityRenderer)
,	m_filter(filter)
{
}

const TypeInfoSet EntityRendererAdapter::getRenderableTypes() const
{
	return m_entityRenderer->getRenderableTypes();
}

void EntityRendererAdapter::setup(
	const world::WorldSetupContext& context
)
{
	m_entityRenderer->setup(context);
}

void EntityRendererAdapter::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
	EntityAdapter* entityAdapter = m_cache->begin(renderable);
	if (!entityAdapter || m_filter(entityAdapter))
	{
		m_entityRenderer->setup(
			context,
			worldRenderView,
			renderable
		);
	}
	m_cache->end();
}

void EntityRendererAdapter::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	EntityAdapter* entityAdapter = m_cache->begin(renderable);
	if (!entityAdapter || m_filter(entityAdapter))
	{
		m_entityRenderer->build(
			context,
			worldRenderView,
			worldRenderPass,
			renderable
		);
	}
	m_cache->end();
}

void EntityRendererAdapter::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	m_entityRenderer->build(
		context,
		worldRenderView,
		worldRenderPass
	);
}

}
