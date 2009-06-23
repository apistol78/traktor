#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include "Physics/Havok/StaticBodyHavok.h"
#include "Physics/Havok/Conversion.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.StaticBodyHavok", StaticBodyHavok, StaticBody)

StaticBodyHavok::StaticBodyHavok(DestroyCallbackHavok* callback, const HvkRef< hkpRigidBody >& rigidBody)
:	BodyHavok< StaticBody >(callback, rigidBody)
{
}

void StaticBodyHavok::setTransform(const Matrix44& transform)
{
	m_rigidBody->setTransform(toHkTransform(transform));
}

Matrix44 StaticBodyHavok::getTransform() const
{
	return fromHkTransform(m_rigidBody->getTransform());
}

void StaticBodyHavok::setActive(bool active)
{
	if (active)
		m_rigidBody->activate();
	else
		m_rigidBody->deactivate();
}

bool StaticBodyHavok::isActive() const
{
	return m_rigidBody->isActive();
}

void StaticBodyHavok::setEnable(bool enable)
{
}

bool StaticBodyHavok::isEnable() const
{
	return true;
}

	}
}
