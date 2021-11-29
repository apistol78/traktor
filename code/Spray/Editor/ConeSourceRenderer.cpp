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
	primitiveRenderer->drawCone(
		translate(coneSource->m_position) * Quaternion(Vector4(0.0f, 0.0f, 1.0f, 0.0f), coneSource->m_normal).toMatrix44(),
		coneSource->m_angle2,
		coneSource->m_angle1,
		1.0f,
		Color4ub(255, 255, 0, 20),
		Color4ub(255, 255, 128, 30)
	);
	primitiveRenderer->drawLine(coneSource->m_position, coneSource->m_position + coneSource->m_normal, Color4ub(255, 255, 0));
}

	}
}
