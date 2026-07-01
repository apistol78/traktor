/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/CullingComponent.h"
#include "World/Entity/CullingRenderer.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.CullingRenderer", 0, CullingRenderer, IEntityRenderer)

bool CullingRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet CullingRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< CullingComponent >();
}

void CullingRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables
)
{
}

void CullingRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables
)
{
	for (Object* renderable : renderables)
	{
		auto cullingComponent = static_cast< CullingComponent* >(renderable);
		cullingComponent->build(context, worldRenderView, worldRenderPass);
	}
}

}
