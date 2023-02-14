/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/CylinderShapeDesc.h"
#include "Physics/Editor/CylinderShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.CylinderShapeRenderer", 0, CylinderShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& CylinderShapeRenderer::getDescType() const
{
	return type_of< CylinderShapeDesc >();
}

void CylinderShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	const CylinderShapeDesc* cylinderShapeDesc = checked_type_cast< const CylinderShapeDesc*, false >(shapeDesc);
	primitiveRenderer->drawWireCylinder(
		(body1Transform[1] * shapeDesc->getLocalTransform()).toMatrix44(),
		cylinderShapeDesc->getRadius(),
		cylinderShapeDesc->getLength(),
		Color4ub(0, 255, 255, 180)
	);
}

}
