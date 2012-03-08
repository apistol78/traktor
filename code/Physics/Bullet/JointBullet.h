#ifndef traktor_physics_JointBullet_H
#define traktor_physics_JointBullet_H

#include <btBulletDynamicsCommon.h>
#include "Core/Ref.h"
#include "Core/Misc/InvokeOnce.h"
#include "Physics/Bullet/BodyBullet.h"
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
	JointBullet(IWorldCallback* callback, Constraint* constraint, BodyBullet* body1, BodyBullet* body2)
	:	m_callback(callback)
	,	m_constraint(constraint)
	,	m_body1(body1)
	,	m_body2(body2)
	,	m_enable(false)
	{
	}

	virtual ~JointBullet()
	{
		destroy();
	}

	virtual void destroy()
	{
		invokeOnce< IWorldCallback, Joint*, btTypedConstraint* >(m_callback, &IWorldCallback::destroyConstraint, this, m_constraint);

		if (m_enable)
		{
			if (m_body1)
				m_body1->removeJoint(this);
			if (m_body2)
				m_body2->removeJoint(this);
		}

		m_constraint = 0;
		m_body1 = 0;
		m_body2 = 0;

		m_enable = false;
	}

	virtual Body* getBody1()
	{
		return m_body1;
	}

	virtual Body* getBody2()
	{
		return m_body2;
	}

	virtual void setEnable(bool enable)
	{
		T_ASSERT (m_callback);

		if (enable == m_enable)
			return;

		if (enable)
		{
			m_body1->addJoint(this);
			m_body2->addJoint(this);
			m_callback->insertConstraint(m_constraint);
		}
		else
		{
			m_body1->removeJoint(this);
			m_body2->removeJoint(this);
			m_callback->removeConstraint(m_constraint);
		}

		m_enable = enable;
	}

	virtual bool isEnable() const
	{
		return m_enable;
	}

protected:
	IWorldCallback* m_callback;
	Constraint* m_constraint;
	Ref< BodyBullet > m_body1;
	Ref< BodyBullet > m_body2;
	bool m_enable;
};

	}
}

#endif	// traktor_physics_JointBullet_H
