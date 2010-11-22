#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Sources/SphereSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SphereSourceRenderer", SphereSourceRenderer, SourceRenderer)

void SphereSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const
{
	SphereSource* sphereSource = checked_type_cast< SphereSource* >(source);

	Vector4 position = sphereSource->getPosition();
	Scalar minRadius = Scalar(sphereSource->getRadius().min);
	Scalar maxRadius = Scalar(sphereSource->getRadius().max);

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f);
		float c1 = std::cos(2.0f * PI * i / 64.0f);
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f);
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f);

		primitiveRenderer->drawLine(
			position + Vector4(c1, 0.0f, s1, 0.0f) * minRadius,
			position + Vector4(c2, 0.0f, s2, 0.0f) * minRadius,
			Color4ub(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			position + Vector4(0.0f, c1, s1, 0.0f) * minRadius,
			position + Vector4(0.0f, c2, s2, 0.0f) * minRadius,
			Color4ub(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			position + Vector4(c1, s1, 0.0f, 0.0f) * minRadius,
			position + Vector4(c2, s2, 0.0f, 0.0f) * minRadius,
			Color4ub(255, 255, 0)
		);

		primitiveRenderer->drawLine(
			position + Vector4(c1, 0.0f, s1, 0.0f) * maxRadius,
			position + Vector4(c2, 0.0f, s2, 0.0f) * maxRadius,
			Color4ub(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			position + Vector4(0.0f, c1, s1, 0.0f) * maxRadius,
			position + Vector4(0.0f, c2, s2, 0.0f) * maxRadius,
			Color4ub(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			position + Vector4(c1, s1, 0.0f, 0.0f) * maxRadius,
			position + Vector4(c2, s2, 0.0f, 0.0f) * maxRadius,
			Color4ub(255, 255, 0)
		);
	}
}

	}
}
