/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/Editor/CapsuleShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.CapsuleShapeRenderer", 0, CapsuleShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& CapsuleShapeRenderer::getDescType() const
{
	return type_of< CapsuleShapeDesc >();
}

void CapsuleShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	const CapsuleShapeDesc* capsuleShapeDesc = checked_type_cast< const CapsuleShapeDesc*, false >(shapeDesc);
	primitiveRenderer->drawWireCylinder(
		(body1Transform[1] * shapeDesc->getLocalTransform()).toMatrix44(),
		capsuleShapeDesc->getRadius(),
		capsuleShapeDesc->getLength(),
		Color4ub(0, 255, 255, 180)
	);
}

	}
}
