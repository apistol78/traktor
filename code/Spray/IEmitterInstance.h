/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Plane.h"
#include "Render/Types.h"
#include "Spray/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderContext;

}

namespace traktor::spray
{

class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! Emitter instance.
 * \ingroup Spray
 */
class T_DLLCLASS IEmitterInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update(Context& context, const Transform& transform, bool emit, bool singleShot) = 0;

	virtual void setup() = 0;

	virtual void render(
		render::handle_t technique,
		render::RenderContext* renderContext,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform,
		const Vector4& cameraPosition,
		const Plane& cameraPlane
	) = 0;

	virtual void synchronize() const = 0;

	virtual Aabb3 getBoundingBox() const = 0;
};

}
#pragma once
