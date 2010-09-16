#ifndef traktor_physics_BodyHavok_H
#define traktor_physics_BodyHavok_H

#include "Physics/Havok/Types.h"

class hkpRigidBody;

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Havok
 */
template < typename Outer >
class BodyHavok : public Outer
{
public:
	BodyHavok(DestroyCallbackHavok* callback, const HvkRef< hkpRigidBody >& rigidBody)
	:	m_callback(callback)
	,	m_rigidBody(rigidBody)
	{
	}

	virtual ~BodyHavok()
	{
		destroy();
	}

	virtual void destroy()
	{
		if (m_callback)
		{
			m_callback->destroyBody(this, m_rigidBody);
			m_callback = 0;
		}

		m_rigidBody.release();

		Outer::destroy();
	}

	inline const HvkRef< hkpRigidBody >& getRigidBody() const { return m_rigidBody; }

protected:
	DestroyCallbackHavok* m_callback;
	HvkRef< hkpRigidBody > m_rigidBody;
};

	}
}

#endif	// traktor_physics_BodyHavok_H
