#include "Spray/Editor/SplineSourceRenderer.h"
#include "Spray/Sources/SplineSource.h"
#include "Render/PrimitiveRenderer.h"
#include "Core/Math/Hermite.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SplineSourceRenderer", SplineSourceRenderer, SourceRenderer)

void SplineSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const
{
	SplineSource* splineSource = checked_type_cast< SplineSource* >(source);

	const AlignedVector< SplineSource::Key >& keys = splineSource->getKeys();
	if (keys.empty())
		return;

	Vector4 position1 = Hermite< SplineSource::Key, Scalar, Vector4 >::evaluate(&keys[0], keys.size(), Scalar(0.0f));
	for (float t = 0.1f; t < keys.back().T; t += 0.1f)
	{
		Vector4 position2 = Hermite< SplineSource::Key, Scalar, Vector4 >::evaluate(&keys[0], keys.size(), Scalar(t));
		primitiveRenderer->drawLine(position1, position2, Color4ub(255, 255, 0));
		position1 = position2;
	}

	for (AlignedVector< SplineSource::Key >::const_iterator i = keys.begin(); i != keys.end(); ++i)
	{
		const Vector4 c_size(0.1f, 0.1f, 0.1f, 0.0f);
		primitiveRenderer->drawWireAabb(Aabb3(i->value - c_size, i->value + c_size), Color4ub(255, 255, 0));
	}
}

	}
}
