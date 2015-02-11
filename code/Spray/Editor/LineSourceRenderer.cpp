#include "Spray/Editor/LineSourceRenderer.h"
#include "Spray/Sources/LineSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.LineSourceRenderer", LineSourceRenderer, SourceRenderer)

void LineSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const Source* source) const
{
	const LineSource* lineSource = checked_type_cast< const LineSource* >(source);

	Vector4 startPosition = lineSource->getStartPosition();
	Vector4 endPosition = lineSource->getEndPosition();

	primitiveRenderer->drawLine(startPosition, endPosition, 1.0f, Color4ub(255, 255, 0));

	int32_t npoints = 2 + lineSource->getSegments();
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
