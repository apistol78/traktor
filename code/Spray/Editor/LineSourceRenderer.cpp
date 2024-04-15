/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Editor/LineSourceRenderer.h"
#include "Spray/Sources/LineSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.LineSourceRenderer", LineSourceRenderer, SourceRenderer)

void LineSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const LineSourceData* lineSource = checked_type_cast< const LineSourceData* >(sourceData);

	const Vector4 startPosition = lineSource->m_startPosition;
	const Vector4 endPosition = lineSource->m_endPosition;

	primitiveRenderer->drawLine(startPosition, endPosition, 1.0f, Color4ub(255, 255, 0));

	const int32_t npoints = 2 + lineSource->m_segments;
	for (int32_t i = 0; i < npoints; ++i)
	{
		const float k = float(i) / (npoints - 1);
		const Vector4 position = lerp(startPosition, endPosition, Scalar(k));

		const Vector4 c_size(0.1f, 0.1f, 0.1f, 0.0f);
		primitiveRenderer->drawWireAabb(Aabb3(position - c_size, position + c_size), 1.0f, Color4ub(255, 255, 0));
	}
}

}
