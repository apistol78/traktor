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
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Plane.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Spray/Types.h"

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace spray
	{

class TrailRenderer;

/*! Ribbon trail instance.
 * \ingroup Spray
 */
class TrailInstance : public Object
{
	T_RTTI_CLASS;

public:
	explicit TrailInstance(
		const resource::Proxy< render::Shader >& shader,
		float width,
		float age,
		float lengthThreshold,
		float breakThreshold
	);

	void update(Context& context, const Transform& transform, bool enable);

	void render(render::handle_t technique, TrailRenderer* trailRenderer, const Transform& transform, const Vector4& cameraPosition, const Plane& cameraPlane);

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

private:
	resource::Proxy< render::Shader > m_shader;
	float m_width;
	float m_age;
	float m_lengthThreshold;
	float m_breakThreshold;
	CircularVector< Vector4, 64 > T_MATH_ALIGN16 m_points;
	Vector4 m_last;
	Vector4 m_time;
	Aabb3 m_boundingBox;
};

	}
}

