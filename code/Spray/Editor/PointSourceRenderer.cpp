#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Sources/PointSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSourceRenderer", PointSourceRenderer, SourceRenderer)

void PointSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const
{
	PointSource* pointSource = checked_type_cast< PointSource* >(source);

	Vector4 position = pointSource->getPosition();

	const Vector4 c_size(0.1f, 0.1f, 0.1f, 0.0f);
	primitiveRenderer->drawWireAabb(Aabb(position - c_size, position + c_size), Color4ub(255, 255, 0));
}

	}
}
