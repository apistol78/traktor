#include <algorithm>
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.Body", Body, Object)

void Body::addCollisionListener(CollisionListener* collisionListener)
{
	m_collisionListeners.push_back(collisionListener);
}

void Body::removeCollisionListener(CollisionListener* collisionListener)
{
	RefArray< CollisionListener >::iterator i = std::find(m_collisionListeners.begin(), m_collisionListeners.end(), collisionListener);
	if (i != m_collisionListeners.end())
		m_collisionListeners.erase(i);
}

void Body::notifyCollisionListeners(const CollisionInfo& collisionInfo)
{
	for (RefArray< CollisionListener >::iterator i = m_collisionListeners.begin(); i != m_collisionListeners.end(); ++i)
		(*i)->notify(collisionInfo);
}

void Body::setUserObject(Object* userObject)
{
	m_userObject = userObject;
}

Ref< Object > Body::getUserObject() const
{
	return m_userObject;
}

	}
}
