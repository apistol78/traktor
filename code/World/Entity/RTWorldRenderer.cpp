/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/RTWorldRenderer.h"

#include "Render/Context/RenderContext.h"
#include "World/Entity/RTWorldComponent.h"
#include "World/WorldBuildContext.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.RTWorldRenderer", 0, RTWorldRenderer, IEntityRenderer)

bool RTWorldRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet RTWorldRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< RTWorldComponent >();
}

void RTWorldRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables
)
{
}

void RTWorldRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables
)
{
	render::RenderContext* renderContext = context.getRenderContext();
	T_ASSERT(renderContext);

	auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"RTWorldRenderer");
	rb->lambda = [=](render::IRenderView* renderView)
	{
		for (Object* renderable : renderables)
		{
			auto rtWorldComponent = static_cast< RTWorldComponent* >(renderable);
			rtWorldComponent->writeAccelerationStructure(renderView);
		}
	};
	renderContext->compute(rb);
}

}
