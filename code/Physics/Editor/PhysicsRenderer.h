/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_PhysicsRenderer_H
#define traktor_physics_PhysicsRenderer_H

#include <map>
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

	namespace resource
	{

class IResourceManager;

	}

	namespace physics
	{

class IPhysicsJointRenderer;
class IPhysicsShapeRenderer;
class JointDesc;
class ShapeDesc;

class T_DLLCLASS PhysicsRenderer : public Object
{
	T_RTTI_CLASS;

public:
	PhysicsRenderer();

	void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const JointDesc* jointDesc
	) const;

	void draw(
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const Transform& body2Transform0,
		const Transform& body2Transform,
		const JointDesc* jointDesc
	) const;

	void draw(
		resource::IResourceManager* resourceManager,
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform& body1Transform0,
		const Transform& body1Transform,
		const ShapeDesc* shapeDesc
	) const;

private:
	std::map< const TypeInfo*, Ref< IPhysicsJointRenderer > > m_jointRenderers;
	std::map< const TypeInfo*, Ref< IPhysicsShapeRenderer > > m_shapeRenderers;
};

	}
}

#endif	// traktor_physics_PhysicsRenderer_H
