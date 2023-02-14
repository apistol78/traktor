/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/JointDesc.h"
#include "Physics/ShapeDesc.h"
#include "Physics/Editor/IPhysicsJointRenderer.h"
#include "Physics/Editor/IPhysicsShapeRenderer.h"
#include "Physics/Editor/PhysicsRenderer.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsRenderer", PhysicsRenderer, Object)

PhysicsRenderer::PhysicsRenderer()
{
	for (auto jointRendererType : type_of< IPhysicsJointRenderer >().findAllOf(false))
	{
		Ref< IPhysicsJointRenderer > jointRenderer = mandatory_non_null_type_cast< IPhysicsJointRenderer* >(jointRendererType->createInstance());
		m_jointRenderers.insert(std::make_pair(&jointRenderer->getDescType(), jointRenderer));
	}
	for (auto shapeRendererType : type_of< IPhysicsShapeRenderer >().findAllOf(false))
	{
		Ref< IPhysicsShapeRenderer > shapeRenderer = mandatory_non_null_type_cast< IPhysicsShapeRenderer* >(shapeRendererType->createInstance());
		m_shapeRenderers.insert(std::make_pair(&shapeRenderer->getDescType(), shapeRenderer));
	}
}

void PhysicsRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform jointTransform[2],
	const Transform body1Transform[2],
	const Transform body2Transform[2],
	const JointDesc* jointDesc
) const
{
	auto it = m_jointRenderers.find(&type_of(jointDesc));
	if (it != m_jointRenderers.end())
		it->second->draw(
			primitiveRenderer,
			jointTransform,
			body1Transform,
			body2Transform,
			jointDesc
		);
}

void PhysicsRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	auto it = m_shapeRenderers.find(&type_of(shapeDesc));
	if (it != m_shapeRenderers.end())
		it->second->draw(
			resourceManager,
			primitiveRenderer,
			body1Transform,
			shapeDesc
		);
}

}
