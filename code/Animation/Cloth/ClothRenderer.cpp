/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothRenderer.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.ClothRenderer", 0, ClothRenderer, world::IEntityRenderer)

bool ClothRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet ClothRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ClothComponent >();
}

void ClothRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables
)
{
}

void ClothRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables
)
{
	for (Object* renderable : renderables)
	{
		auto clothEntity = mandatory_non_null_type_cast< ClothComponent* >(renderable);
		clothEntity->build(context, worldRenderView, worldRenderPass);
	}
}

}
