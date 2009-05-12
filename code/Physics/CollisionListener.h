#ifndef traktor_physics_CollisionListener_H
#define traktor_physics_CollisionListener_H

#include <vector>
#include "Core/Object.h"
#include "Core/Heap/HeapNew.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \ingroup Physics */
//@{

class Body;

struct CollisionContact
{
	Vector4 position;
	Vector4 normal;
	float depth;
};

struct CollisionInfo
{
	Body* body1;
	Body* body2;
	AlignedVector< CollisionContact > contacts;
};

class T_DLLCLASS CollisionListener : public Object
{
	T_RTTI_CLASS(CollisionListener)

public:
	virtual void notify(const CollisionInfo& collisionInfo) = 0;
};

template < typename TargetType >
class MethodCollisionListener : public CollisionListener
{
public:
	typedef void (TargetType::*TargetMethod)(const CollisionInfo& collisionInfo);

	MethodCollisionListener(TargetType* object, TargetMethod method)
	:	m_object(object)
	,	m_method(method)
	{
	}

	virtual void notify(const CollisionInfo& collisionInfo)
	{
		(m_object->*m_method)(collisionInfo);
	}

private:
	TargetType* m_object;
	TargetMethod m_method;
};

template < typename TargetType >
inline CollisionListener* createCollisionListener(TargetType* object, typename MethodCollisionListener< TargetType >::TargetMethod method)
{
	return gc_new< MethodCollisionListener< TargetType > >(object, method);
}

//@}

	}
}

#endif	// traktor_physics_CollisionListener_H
