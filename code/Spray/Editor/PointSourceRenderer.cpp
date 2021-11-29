#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Sources/PointSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSourceRenderer", PointSourceRenderer, SourceRenderer)

void PointSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const PointSourceData* pointSource = checked_type_cast< const PointSourceData* >(sourceData);
	primitiveRenderer->drawSolidPoint(pointSource->m_position, 8.0f, Color4ub(255, 255, 0, 180));
}

	}
}
