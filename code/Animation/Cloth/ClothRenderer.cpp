/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothRenderer.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothRenderer", ClothRenderer, world::IEntityRenderer)

const TypeInfoSet ClothRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ClothComponent >();
}

void ClothRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void ClothRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void ClothRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto clothEntity = mandatory_non_null_type_cast< ClothComponent* >(renderable);
	clothEntity->build(context, worldRenderView, worldRenderPass);
}

void ClothRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}
