#include "Spray/PointSet.h"
#include "Spray/Editor/PointSetSourceRenderer.h"
#include "Spray/Sources/PointSetSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSetSourceRenderer", PointSetSourceRenderer, SourceRenderer)

void PointSetSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const Source* source) const
{
	const PointSetSource* pointSetSource = checked_type_cast< const PointSetSource* >(source);

	const resource::Proxy< PointSet >& pointSet = pointSetSource->getPointSet();
	if (!pointSet)
		return;

	const AlignedVector< PointSet::Point >& points = pointSet->get();
	for (AlignedVector< PointSet::Point >::const_iterator i = points.begin(); i != points.end(); ++i)
		primitiveRenderer->drawSolidPoint(i->position + pointSetSource->getOffset(), 3.0f, Color4ub(255, 255, 0));
}

	}
}
