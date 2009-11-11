#ifndef traktor_physics_JointOde_H
#define traktor_physics_JointOde_H

#include "Core/Heap/Ref.h"
#include "Physics/Ode/Types.h"

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup ODE
 */
template < typename Outer >
class JointOde : public Outer
{
public:
	JointOde(DestroyCallback* callback, dJointID jointId, Body* body1, Body* body2)
	:	m_callback(callback)
	,	m_jointId(jointId)
	,	m_body1(body1)
	,	m_body2(body2)
	{
	}

	virtual ~JointOde()
	{
		destroy();
	}

	virtual void destroy()
	{
		if (m_callback)
		{
			m_callback->jointDestroyed(this);
			m_callback = 0;
		}
		if (m_jointId)
		{
			dJointDestroy(m_jointId);
			m_jointId = 0;
		}
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
	dJointID m_jointId;
	Ref< Body > m_body1;
	Ref< Body > m_body2;
};

	}
}

#endif	// traktor_physics_JointOde_H
