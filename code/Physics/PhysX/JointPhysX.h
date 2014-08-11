#ifndef traktor_physics_JointPhysX_H
#define traktor_physics_JointPhysX_H

#include "Physics/PhysX/Types.h"

namespace physx
{

class PxJoint;

}

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup PhysX
 */
template < typename Outer >
class JointPhysX : public Outer
{
public:
	JointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2)
	:	m_callback(callback)
	,	m_joint(joint)
	,	m_body1(body1)
	,	m_body2(body2)
	{
	}

	virtual ~JointPhysX()
	{
		destroy();
	}

	virtual void destroy()
	{
		if (m_callback && m_joint)
		{
			m_callback->destroyJoint(this, m_joint);
			m_callback = 0;
		}
		m_joint = 0;
	}

	virtual Body* getBody1()
	{
		return m_body1;
	}

	virtual Body* getBody2()
	{
		return m_body2;
	}

	virtual void* getInternal()
	{
		return m_joint;
	}

protected:
	IWorldCallback* m_callback;
	physx::PxJoint* m_joint;
	Ref< Body > m_body1;
	Ref< Body > m_body2;
};

	}
}

#endif	// traktor_physics_JointPhysX_H
