/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Clouds/CloudRenderer.h"
#include "World/WorldBuildContext.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudRenderer", CloudRenderer, world::IEntityRenderer)

CloudRenderer::CloudRenderer(render::PrimitiveRenderer* primitiveRenderer)
:	m_primitiveRenderer(primitiveRenderer)
{
}

const TypeInfoSet CloudRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< CloudComponent >();
}

void CloudRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void CloudRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void CloudRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto cloudComponent = mandatory_non_null_type_cast< CloudComponent* >(renderable);
	cloudComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass, m_primitiveRenderer);
}

void CloudRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}
