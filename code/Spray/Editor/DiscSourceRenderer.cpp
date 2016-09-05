#include "Core/Math/Const.h"
#include "Spray/Editor/DiscSourceRenderer.h"
#include "Spray/Sources/DiscSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.DiscSourceRenderer", DiscSourceRenderer, SourceRenderer)

void DiscSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const DiscSourceData* discSource = checked_type_cast< const DiscSourceData* >(sourceData);

	Vector4 position = discSource->m_position;
	Vector4 normal = discSource->m_normal;
	Scalar minRadius = Scalar(discSource->m_radius.min);
	Scalar maxRadius = Scalar(discSource->m_radius.max);

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

		primitiveRenderer->drawLine(position + d1 * minRadius, position + d2 * minRadius, Color4ub(255, 255, 0));
		primitiveRenderer->drawLine(position + d1 * maxRadius, position + d2 * maxRadius, Color4ub(255, 255, 0));
	}

	primitiveRenderer->drawLine(position, position + normal, Color4ub(255, 255, 0));
}

	}
}
