#ifndef traktor_physics_JointHavok_H
#define traktor_physics_JointHavok_H

#include <Physics/Dynamics/Constraint/hkpConstraintInstance.h>
#include "Physics/Havok/Types.h"

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Havok
 */
template < typename Outer >
class JointHavok : public Outer
{
public:
	JointHavok(DestroyCallbackHavok* callback, const HvkRef< hkpConstraintInstance >& constraint, Body* body1, Body* body2)
	:	m_callback(callback)
	,	m_constraint(constraint)
	,	m_body1(body1)
	,	m_body2(body2)
	{
	}

	virtual ~JointHavok()
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
		return m_constraint;
	}

protected:
	DestroyCallbackHavok* m_callback;
	HvkRef< hkpConstraintInstance > m_constraint;
	Ref< Body > m_body1;
	Ref< Body > m_body2;
};

	}
}

#endif	// traktor_physics_JointHavok_H
