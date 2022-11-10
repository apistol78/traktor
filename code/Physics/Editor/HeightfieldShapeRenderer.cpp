/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Heightfield.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Editor/HeightfieldShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldShapeRenderer", 0, HeightfieldShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& HeightfieldShapeRenderer::getDescType() const
{
	return type_of< HeightfieldShapeDesc >();
}

void HeightfieldShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	const HeightfieldShapeDesc* heightfieldShapeDesc = checked_type_cast< const HeightfieldShapeDesc*, false >(shapeDesc);

	resource::Proxy< hf::Heightfield > heightfield;
	if (resourceManager->bind(heightfieldShapeDesc->getHeightfield(), heightfield))
	{
		const Vector4& extent = heightfield->getWorldExtent();
		Aabb3 boundingBox(-extent / Scalar(2.0f), extent / Scalar(2.0f));
		primitiveRenderer->drawWireAabb(boundingBox, 1.0f, Color4ub(0, 255, 255, 180));
	}
}

	}
}
