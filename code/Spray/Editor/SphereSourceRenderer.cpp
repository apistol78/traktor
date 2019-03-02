#include "Render/PrimitiveRenderer.h"
#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Sources/SphereSourceData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SphereSourceRenderer", SphereSourceRenderer, SourceRenderer)

void SphereSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const SphereSourceData* sphereSource = checked_type_cast< const SphereSourceData* >(sourceData);

	Vector4 position = sphereSource->m_position;
	float minRadius = sphereSource->m_radius.min;
	float maxRadius = sphereSource->m_radius.max;

	Matrix44 T = translate(position);

	primitiveRenderer->drawWireSphere(T, minRadius, Color4ub(255, 255, 0));
	primitiveRenderer->drawWireSphere(T, maxRadius, Color4ub(255, 255, 0));
}

	}
}
