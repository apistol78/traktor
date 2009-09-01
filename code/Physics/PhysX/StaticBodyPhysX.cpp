#include <NxPhysics.h>
#include "Physics/PhysX/StaticBodyPhysX.h"
#include "Physics/PhysX/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.StaticBodyPhysX", StaticBodyPhysX, StaticBody)

StaticBodyPhysX::StaticBodyPhysX(DestroyCallbackPhysX* callback, NxActor* actor)
:	BodyPhysX< StaticBody >(callback, actor)
{
}

void StaticBodyPhysX::setTransform(const Transform& transform)
{
	m_actor->setGlobalPose(toNxMat34(transform));
}

Transform StaticBodyPhysX::getTransform() const
{
	return fromNxMat34(m_actor->getGlobalPose());
}

void StaticBodyPhysX::setActive(bool active)
{
}

bool StaticBodyPhysX::isActive() const
{
	return true;
}

void StaticBodyPhysX::setEnable(bool enable)
{
}

bool StaticBodyPhysX::isEnable() const
{
	return true;
}

	}
}
