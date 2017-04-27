/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const ShapeDesc* shapeDesc
) const
{
	const SphereShapeDesc* sphereShapeDesc = checked_type_cast< const SphereShapeDesc*, false >(shapeDesc);
	primitiveRenderer->drawWireSphere(
		(body1Transform * shapeDesc->getLocalTransform()).toMatrix44(),
		sphereShapeDesc->getRadius(),
		Color4ub(0, 255, 255, 180)
	);
}

	}
}
