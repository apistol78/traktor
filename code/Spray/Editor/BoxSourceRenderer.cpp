#include "Spray/Editor/BoxSourceRenderer.h"
#include "Spray/Sources/BoxSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.BoxSourceRenderer", BoxSourceRenderer, SourceRenderer)

void BoxSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const
{
	BoxSource* boxSource = checked_type_cast< BoxSource* >(source);

	Vector4 position = boxSource->getPosition();
	Vector4 extent = boxSource->getExtent();

	primitiveRenderer->drawWireAabb(position, extent, Color(255, 255, 0));
}

	}
}
