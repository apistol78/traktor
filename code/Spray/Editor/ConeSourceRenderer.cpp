#include "Spray/Editor/ConeSourceRenderer.h"
#include "Spray/Sources/ConeSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.ConeSourceRenderer", ConeSourceRenderer, SourceRenderer)

void ConeSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const
{
	ConeSource* coneSource = checked_type_cast< ConeSource* >(source);

	Vector4 position = coneSource->getPosition();
	Vector4 normal = coneSource->getNormal();
	float angle1 = coneSource->getAngle1();
	float angle2 = coneSource->getAngle2();

	for (int i = 0; i < 20; ++i)
	{
		primitiveRenderer->drawCone(
			translate(position) * Quaternion(Vector4(0.0f, 0.0f, 1.0f, 0.0f), normal).toMatrix44(),
			angle2,
			angle1,
			1.0f,
			Color(255, 255, 0, 16),
			Color(255, 255, 255, 32)
		);
	}

	primitiveRenderer->drawLine(position, position + normal, Color(255, 255, 0));
}

	}
}
