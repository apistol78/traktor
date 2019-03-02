#include "Render/PrimitiveRenderer.h"
#include "Spray/Editor/QuadSourceRenderer.h"
#include "Spray/Sources/QuadSourceData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.QuadSourceRenderer", QuadSourceRenderer, SourceRenderer)

void QuadSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const QuadSourceData* quadSource = checked_type_cast< const QuadSourceData* >(sourceData);

	Vector4 center = quadSource->m_center;
	Vector4 axis1 = quadSource->m_axis1;
	Vector4 axis2 = quadSource->m_axis2;
	Vector4 normal = quadSource->m_normal;

	Vector4 corners[] =
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
}

	}
}
