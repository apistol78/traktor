#ifndef traktor_physics_JointBullet_H
#define traktor_physics_JointBullet_H

#include <btBulletDynamicsCommon.h>
#include "Core/Heap/Ref.h"
#include "Physics/Bullet/Types.h"

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
struct JointSolver
{
	virtual void prepare() {}

	virtual void update(float deltaTime) {}
};

/*!
 * \ingroup Bullet
 */
template < typename Outer, typename Constraint >
class JointBullet
:	public Outer
,	public JointSolver
{
public:
	JointBullet(DestroyCallback* callback, Constraint* constraint, Body* body1, Body* body2)
	:	m_callback(callback)
	,	m_constraint(constraint)
	,	m_body1(body1)
	,	m_body2(body2)
	{
	}

	virtual ~JointBullet()
	{
		destroy();
	}

	virtual void destroy()
	{
		if (m_callback)
		{
			m_callback->destroyJoint(this, m_constraint);
			m_callback = 0;
		}
		m_constraint = 0;
	}

	virtual Ref< Body > getBody1()
	{
		return m_body1;
	}

	virtual Ref< Body > getBody2()
	{
		return m_body2;
	}

protected:
	DestroyCallback* m_callback;
	Constraint* m_constraint;
	Ref< Body > m_body1;
	Ref< Body > m_body2;
};

	}
}

#endif	// traktor_physics_JointBullet_H
