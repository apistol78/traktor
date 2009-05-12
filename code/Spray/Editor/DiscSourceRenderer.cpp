#include "Spray/Editor/DiscSourceRenderer.h"
#include "Spray/Sources/DiscSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.DiscSourceRenderer", DiscSourceRenderer, SourceRenderer)

void DiscSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const
{
	DiscSource* discSource = checked_type_cast< DiscSource* >(source);

	Vector4 position = discSource->getPosition();
	Vector4 normal = discSource->getNormal();
	Scalar minRadius = Scalar(discSource->getRadius().min);
	Scalar maxRadius = Scalar(discSource->getRadius().max);

	const Vector4 c_axisZ(0.0f, 0.0f, 1.0f, 0.0f);

	Vector4 axisX = cross(normal, c_axisZ).normalized();
	Vector4 axisZ = cross(axisX, normal).normalized();

	for (int i = 0; i < 20; ++i)
	{
		float a1 = float(i / 20.0f) * PI * 2.0f;
		float a2 = float((i + 1) / 20.0f) * PI * 2.0f;
		float s1 = sinf(a1), s2 = sinf(a2);
		float c1 = cosf(a1), c2 = cosf(a2);

		Vector4 d1 = Scalar(c1) * axisZ + Scalar(s1) * axisX;
		Vector4 d2 = Scalar(c2) * axisZ + Scalar(s2) * axisX;

		primitiveRenderer->drawLine(position + d1 * minRadius, position + d2 * minRadius, Color(255, 255, 0));
		primitiveRenderer->drawLine(position + d1 * maxRadius, position + d2 * maxRadius, Color(255, 255, 0));
	}

	primitiveRenderer->drawLine(position, position + normal, Color(255, 255, 0));
}

	}
}
