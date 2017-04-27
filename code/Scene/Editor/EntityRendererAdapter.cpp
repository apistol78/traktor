/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"

namespace traktor
{
	namespace scene
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityRendererAdapter", EntityRendererAdapter, world::IEntityRenderer)

EntityRendererAdapter::EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer)
:	m_cache(cache)
,	m_entityRenderer(entityRenderer)
{
}

const TypeInfoSet EntityRendererAdapter::getRenderableTypes() const
{
	return m_entityRenderer->getRenderableTypes();
}

void EntityRendererAdapter::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	EntityAdapter* entityAdapter = m_cache->begin(renderable);
	if (!entityAdapter || entityAdapter->isVisible())
	{
		m_entityRenderer->render(
			worldContext,
			worldRenderView,
			worldRenderPass,
			renderable
		);
	}
	m_cache->end();
}

void EntityRendererAdapter::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	m_entityRenderer->flush(
		worldContext,
		worldRenderView,
		worldRenderPass
	);
}
	
	}
}
