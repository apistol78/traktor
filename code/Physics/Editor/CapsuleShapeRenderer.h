/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/Editor/IPhysicsShapeRenderer.h"

namespace traktor
{
	namespace physics
	{

class CapsuleShapeRenderer : public IPhysicsShapeRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo& getDescType() const override final;

	virtual void draw(
		resource::IResourceManager* resourceManager,
		render::PrimitiveRenderer* primitiveRenderer,
		const Transform body1Transform[2],
		const ShapeDesc* shapeDesc
	) const override final;
};

	}
}

