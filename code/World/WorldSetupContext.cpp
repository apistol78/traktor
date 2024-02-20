/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/IEntityRenderer.h"
#include "World/WorldSetupContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldSetupContext", WorldSetupContext, Object)

WorldSetupContext::WorldSetupContext(
	const World* world,
	const WorldEntityRenderers* entityRenderers,
	const IrradianceGrid* irradianceGrid,
	render::RenderGraph& renderGraph,
	AlignedVector< render::handle_t >& visualAttachments
)
:	m_world(world)
,	m_entityRenderers(entityRenderers)
,	m_irradianceGrid(irradianceGrid)
,	m_renderGraph(renderGraph)
,	m_visualAttachments(visualAttachments)
{
	m_visualAttachments.resize(0);
}

}
