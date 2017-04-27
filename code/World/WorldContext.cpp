/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Context/RenderContext.h"
#include "World/Entity.h"
#include "World/IEntityRenderer.h"
#include "World/WorldContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const uint32_t c_renderContextSize = 4 * 1024 * 1024;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldContext", WorldContext, Object)

WorldContext::WorldContext(WorldEntityRenderers* entityRenderers)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(new render::RenderContext(c_renderContextSize))
,	m_lastRenderableType(0)
,	m_lastRenderer(0)
{
}

void WorldContext::clear()
{
	if (m_renderContext)
		m_renderContext->flush();
}

void WorldContext::build(WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass, Object* renderable)
{
	if (!renderable)
		return;

	IEntityRenderer* renderer = 0;

	const TypeInfo& renderableType = type_of(renderable);
	if (m_lastRenderableType == &renderableType)
	{
		// Fast path, no need to lookup new entity renderer as it's the same as last entity rendered.
		renderer = m_lastRenderer;
	}
	else
	{
		// Need to find entity renderer.
		renderer = m_entityRenderers->find(renderableType);
		m_lastRenderableType = &renderableType;
		m_lastRenderer = renderer;
	}

	if (renderer)
		renderer->render(*this, worldRenderView, worldRenderPass, renderable);
}

void WorldContext::flush(WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass)
{
	T_ASSERT (m_entityRenderers);
	const RefArray< IEntityRenderer >& entityRenderers = m_entityRenderers->get();
	for (RefArray< IEntityRenderer >::const_iterator i = entityRenderers.begin(); i != entityRenderers.end(); ++i)
		(*i)->flush(*this, worldRenderView, worldRenderPass);
}

	}
}
