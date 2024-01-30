/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimatedMeshComponentRenderer.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponentRenderer", AnimatedMeshComponentRenderer, mesh::MeshComponentRenderer)

const TypeInfoSet AnimatedMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< AnimatedMeshComponent >();
}

}
