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
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Math/Aabb3.h"
#include "Shape/Editor/Bake/Types.h"
#include "Model/Model.h"

namespace traktor
{

class Transform;

}

namespace traktor::shape
{

class GBuffer : public Object
{
	T_RTTI_CLASS;

public:
	struct Element
	{
		Vector4 position;
		Vector4 normal;
		Vector4 tangent;
		uint32_t polygon;
		uint32_t material;
		float distance;
	};

	typedef StaticVector< Element, 4 > element_vector_t;

	bool create(int32_t width, int32_t height, const model::Model& model, const Transform& transform, uint32_t texCoordChannel, uint32_t maxElements);

	const element_vector_t& get(int32_t x, int32_t y) const { return m_data[x + y * m_width]; }

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

private:
	int32_t m_width = 0;
	int32_t m_height = 0;
	AlignedVector< element_vector_t > m_data;
	Aabb3 m_boundingBox;
};

}
