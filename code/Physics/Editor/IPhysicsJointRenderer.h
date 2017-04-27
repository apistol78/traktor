/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_IPhysicsJointRenderer_H
#define traktor_physics_IPhysicsJointRenderer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

	namespace render
	{

class PrimitiveRenderer;

	}

	namespace physics
	{

class JointDesc;

class IPhysicsJointRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const = 0;

	virtual void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const JointDesc* jointDesc
	) const = 0;

	virtual void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const Transform& body2Transform0,
		const Transform& body2Transform,
		const JointDesc* jointDesc
	) const = 0;
};

	}
}

#endif	// traktor_physics_IPhysicsJointRenderer_H
