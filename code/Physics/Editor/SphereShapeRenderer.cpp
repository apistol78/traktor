/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/SphereShapeDesc.h"
#include "Physics/Editor/SphereShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.SphereShapeRenderer", 0, SphereShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& SphereShapeRenderer::getDescType() const
{
	return type_of< SphereShapeDesc >();
}

void SphereShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	const SphereShapeDesc* sphereShapeDesc = checked_type_cast< const SphereShapeDesc*, false >(shapeDesc);
	primitiveRenderer->drawWireSphere(
		(body1Transform[1] * shapeDesc->getLocalTransform()).toMatrix44(),
		sphereShapeDesc->getRadius(),
		Color4ub(0, 255, 255, 180)
	);
}

	}
}
