/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/PrimitiveRenderer.h"
#include "Spray/Editor/QuadSourceRenderer.h"
#include "Spray/Sources/QuadSourceData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.QuadSourceRenderer", QuadSourceRenderer, SourceRenderer)

void QuadSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const QuadSourceData* quadSource = checked_type_cast< const QuadSourceData* >(sourceData);

	const Vector4 center = quadSource->m_center;
	const Vector4 axis1 = quadSource->m_axis1;
	const Vector4 axis2 = quadSource->m_axis2;
	const Vector4 normal = quadSource->m_normal;

	const Vector4 corners[] =
	{
		center - axis1 - axis2,
		center + axis1 - axis2,
		center + axis1 + axis2,
		center - axis1 + axis2
	};

	primitiveRenderer->drawWireQuad(corners[0], corners[1], corners[2], corners[3], Color4ub(255, 255, 0));

	primitiveRenderer->drawLine(corners[0], corners[2], Color4ub(255, 255, 0));
	primitiveRenderer->drawLine(corners[1], corners[3], Color4ub(255, 255, 0));

	primitiveRenderer->drawLine(center, center + normal, Color4ub(255, 255, 0));
	primitiveRenderer->drawArrowHead(center + normal, center + normal * 1.2_simd, 0.8f, Color4ub(255, 255, 0));
}

}
