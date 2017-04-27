/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/BoxShapeDesc.h"
#include "Physics/Editor/BoxShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BoxShapeRenderer", 0, BoxShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& BoxShapeRenderer::getDescType() const
{
	return type_of< BoxShapeDesc >();
}

void BoxShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const ShapeDesc* shapeDesc
) const
{
	const BoxShapeDesc* boxShapeDesc = checked_type_cast< const BoxShapeDesc*, false >(shapeDesc);

	Aabb3 boundingBox(-boxShapeDesc->getExtent(), boxShapeDesc->getExtent());

	primitiveRenderer->pushWorld((body1Transform * shapeDesc->getLocalTransform()).toMatrix44());

	primitiveRenderer->drawSolidAabb(boundingBox, Color4ub(128, 255, 255, 128));
	primitiveRenderer->drawWireAabb(boundingBox, Color4ub(0, 255, 255));

	primitiveRenderer->popWorld();
}

	}
}
