#include "Render/PrimitiveRenderer.h"
#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Sources/SphereSource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SphereSourceRenderer", SphereSourceRenderer, SourceRenderer)

void SphereSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const Source* source) const
{
	const SphereSource* sphereSource = checked_type_cast< const SphereSource* >(source);

	Vector4 position = sphereSource->getPosition();
	float minRadius = sphereSource->getRadius().min;
	float maxRadius = sphereSource->getRadius().max;

	Matrix44 T = translate(position);

	primitiveRenderer->drawWireSphere(T, minRadius, Color4ub(255, 255, 0));
	primitiveRenderer->drawWireSphere(T, maxRadius, Color4ub(255, 255, 0));
}

	}
}
