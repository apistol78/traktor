#ifndef traktor_physics_BodyPhysX_H
#define traktor_physics_BodyPhysX_H

#include "Physics/PhysX/Types.h"

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup PhysX
 */
template < typename Outer >
class BodyPhysX : public Outer
{
public:
	BodyPhysX(DestroyCallbackPhysX* callback, NxActor* actor)
	:	m_callback(callback)
	,	m_actor(actor)
	{
	}

	virtual ~BodyPhysX()
	{
		destroy();
	}

	virtual void destroy()
	{
		if (m_callback)
		{
			m_callback->destroyBody(this, *m_actor);
			m_callback = 0;
		}
		m_actor = 0;
		Outer::destroy();
	}

	inline NxActor* getActor() const { return m_actor; }

protected:
	DestroyCallbackPhysX* m_callback;
	NxActor* m_actor;
};

	}
}

#endif	// traktor_physics_BodyPhysX_H
