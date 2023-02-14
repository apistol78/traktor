/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"

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

}

namespace traktor::render
{

class PrimitiveRenderer;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::physics
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
		const Transform jointTransform[2],
		const Transform body1Transform[2],
		const Transform body2Transform[2],
		const JointDesc* jointDesc
	) const;

	void draw(
		resource::IResourceManager* resourceManager,
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform body1Transform[2],
		const ShapeDesc* shapeDesc
	) const;

private:
	SmallMap< const TypeInfo*, Ref< IPhysicsJointRenderer > > m_jointRenderers;
	SmallMap< const TypeInfo*, Ref< IPhysicsShapeRenderer > > m_shapeRenderers;
};

}
