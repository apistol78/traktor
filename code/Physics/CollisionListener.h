/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_CollisionListener_H
#define traktor_physics_CollisionListener_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	int32_t material1;
	int32_t material2;
};

struct CollisionInfo
{
	Body* body1;
	Body* body2;
	AlignedVector< CollisionContact > contacts;
};

class T_DLLCLASS CollisionListener : public Object
{
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
inline Ref< CollisionListener > createCollisionListener(TargetType* object, typename MethodCollisionListener< TargetType >::TargetMethod method)
{
	return new MethodCollisionListener< TargetType >(object, method);
}

//@}

	}
}

#endif	// traktor_physics_CollisionListener_H
