/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual ~JointSolver() {}

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
	{
		if (m_body1)
			m_body1->addConstraint(m_constraint);
		if (m_body2)
			m_body2->addConstraint(m_constraint);
	}

	virtual ~JointBullet()
	{
		destroy();
	}

	virtual void destroy() T_OVERRIDE
	{
		if (m_body1)
			m_body1->removeConstraint(m_constraint);
		if (m_body2)
			m_body2->removeConstraint(m_constraint);

		invokeOnce< IWorldCallback, Joint*, btTypedConstraint* >(m_callback, &IWorldCallback::destroyConstraint, this, m_constraint);

		m_constraint = 0;
		m_body1 = 0;
		m_body2 = 0;
	}

	virtual Body* getBody1() T_OVERRIDE T_FINAL
	{
		return m_body1;
	}

	virtual Body* getBody2() T_OVERRIDE T_FINAL
	{
		return m_body2;
	}

	virtual void* getInternal() T_OVERRIDE T_FINAL
	{
		return m_constraint;
	}

protected:
	IWorldCallback* m_callback;
	Constraint* m_constraint;
	Ref< BodyBullet > m_body1;
	Ref< BodyBullet > m_body2;
};

	}
}

#endif	// traktor_physics_JointBullet_H
