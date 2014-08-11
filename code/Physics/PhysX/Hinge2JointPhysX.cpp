#include <extensions\PxRevoluteJoint.h>
#include "Core/Math/Const.h"
#include "Physics/PhysX/Hinge2JointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Hinge2JointPhysX", Hinge2JointPhysX, Hinge2Joint)

Hinge2JointPhysX::Hinge2JointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< Hinge2Joint >(callback, joint, body1, body2)
{
}

void Hinge2JointPhysX::addTorques(float torqueAxis1, float torqueAxis2)
{
}

float Hinge2JointPhysX::getAngleAxis1() const
{
	return 0.0f;
}

void Hinge2JointPhysX::setVelocityAxis1(float velocityAxis1)
{
}

void Hinge2JointPhysX::setVelocityAxis2(float velocityAxis2)
{
}

void Hinge2JointPhysX::getAnchors(Vector4& outAnchor1, Vector4& outAnchor2) const
{
}

	}
}
