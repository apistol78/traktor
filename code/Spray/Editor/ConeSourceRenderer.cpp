/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spray/Editor/ConeSourceRenderer.h"
#include "Spray/Sources/ConeSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.ConeSourceRenderer", ConeSourceRenderer, SourceRenderer)

void ConeSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const ConeSourceData* coneSource = checked_type_cast< const ConeSourceData* >(sourceData);

	float angle1 = asinf(coneSource->m_angle1);
	float angle2 = asinf(coneSource->m_angle2);

	for (int i = 0; i < 20; ++i)
	{
		primitiveRenderer->drawCone(
			translate(coneSource->m_position) * Quaternion(Vector4(0.0f, 0.0f, 1.0f, 0.0f), coneSource->m_normal).toMatrix44(),
			angle2,
			angle1,
			1.0f,
			Color4ub(255, 255, 0, 16),
			Color4ub(255, 255, 255, 32)
		);
	}

	primitiveRenderer->drawLine(coneSource->m_position, coneSource->m_position + coneSource->m_normal, Color4ub(255, 255, 0));
}

	}
}
