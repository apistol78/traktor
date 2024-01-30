/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/IEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldBuildContext", WorldBuildContext, Object)

WorldBuildContext::WorldBuildContext(const WorldEntityRenderers* entityRenderers, render::RenderContext* renderContext)
:	m_entityRenderers(entityRenderers)
,	m_renderContext(renderContext)
{
}

}
