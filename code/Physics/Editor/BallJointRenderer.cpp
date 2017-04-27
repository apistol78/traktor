/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/BallJointDesc.h"
#include "Physics/Editor/BallJointRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BallJointRenderer", 0, BallJointRenderer, IPhysicsJointRenderer)

const TypeInfo& BallJointRenderer::getDescType() const
{
	return type_of< BallJointDesc >();
}

void BallJointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const JointDesc* jointDesc
) const
{
	const BallJointDesc* ballJointDesc = checked_type_cast< const BallJointDesc*, false >(jointDesc);
	Vector4 jointAnchorW = ballJointDesc->getAnchor().xyz1();
	primitiveRenderer->drawSolidPoint(jointAnchorW, 4.0f, Color4ub(255, 255, 255));
}

void BallJointRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const Transform& body2Transform0,
	const Transform& body2Transform,
	const JointDesc* jointDesc
) const
{
	const BallJointDesc* ballJointDesc = checked_type_cast< const BallJointDesc*, false >(jointDesc);
	Vector4 jointAnchor = body1Transform0.inverse() * ballJointDesc->getAnchor().xyz1();
	Vector4 jointAnchorW = body1Transform * jointAnchor;
	primitiveRenderer->drawSolidPoint(jointAnchorW, 4.0f, Color4ub(255, 255, 255));
}

	}
}
