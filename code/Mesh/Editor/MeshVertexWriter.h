/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace mesh
	{

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const float* data);

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const Vector2& data);

uint32_t writeVertexData(const AlignedVector< render::VertexElement >& vertexElements, uint8_t* vertex, render::DataUsage usage, uint32_t index, const Vector4& data);

	}
}

