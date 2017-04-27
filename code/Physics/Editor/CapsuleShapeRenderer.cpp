/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const ShapeDesc* shapeDesc
) const
{
	const CapsuleShapeDesc* capsuleShapeDesc = checked_type_cast< const CapsuleShapeDesc*, false >(shapeDesc);
	primitiveRenderer->drawWireCylinder(
		(body1Transform * shapeDesc->getLocalTransform()).toMatrix44(),
		capsuleShapeDesc->getRadius(),
		capsuleShapeDesc->getLength(),
		Color4ub(0, 255, 255, 180)
	);
}

	}
}
