#include "Spray/Editor/BoxSourceRenderer.h"
#include "Spray/Sources/BoxSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.BoxSourceRenderer", BoxSourceRenderer, SourceRenderer)

void BoxSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const Source* source) const
{
	const BoxSource* boxSource = checked_type_cast< const BoxSource* >(source);

	Vector4 position = boxSource->getPosition();
	Vector4 extent = boxSource->getExtent();

	primitiveRenderer->drawWireAabb(position, extent, Color4ub(255, 255, 0));
}

	}
}
