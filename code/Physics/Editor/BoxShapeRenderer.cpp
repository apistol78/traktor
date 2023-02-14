/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/BoxShapeDesc.h"
#include "Physics/Editor/BoxShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BoxShapeRenderer", 0, BoxShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& BoxShapeRenderer::getDescType() const
{
	return type_of< BoxShapeDesc >();
}

void BoxShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	const BoxShapeDesc* boxShapeDesc = checked_type_cast< const BoxShapeDesc*, false >(shapeDesc);
	const Vector4 margin(Scalar(boxShapeDesc->getMargin()));
	const Aabb3 boundingBox(-boxShapeDesc->getExtent() - margin, boxShapeDesc->getExtent() + margin);

	primitiveRenderer->pushWorld((body1Transform[1] * shapeDesc->getLocalTransform()).toMatrix44());

	primitiveRenderer->drawSolidAabb(boundingBox, Color4ub(128, 255, 255, 128));
	primitiveRenderer->drawWireAabb(boundingBox, 1.0f, Color4ub(0, 255, 255));

	primitiveRenderer->popWorld();
}

}
