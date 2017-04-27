/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spray/Editor/LineSourceRenderer.h"
#include "Spray/Sources/LineSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.LineSourceRenderer", LineSourceRenderer, SourceRenderer)

void LineSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const LineSourceData* lineSource = checked_type_cast< const LineSourceData* >(sourceData);

	Vector4 startPosition = lineSource->m_startPosition;
	Vector4 endPosition = lineSource->m_endPosition;

	primitiveRenderer->drawLine(startPosition, endPosition, 1.0f, Color4ub(255, 255, 0));

	int32_t npoints = 2 + lineSource->m_segments;
	for (int32_t i = 0; i < npoints; ++i)
	{
		float k = float(i) / (npoints - 1);
		Vector4 position = lerp(startPosition, endPosition, Scalar(k));

		const Vector4 c_size(0.1f, 0.1f, 0.1f, 0.0f);
		primitiveRenderer->drawWireAabb(Aabb3(position - c_size, position + c_size), Color4ub(255, 255, 0));
	}
}

	}
}
