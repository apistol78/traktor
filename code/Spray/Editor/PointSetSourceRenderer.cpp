#include "Spray/Editor/PointSetSourceRenderer.h"
#include "Spray/Sources/PointSetSource.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSetSourceRenderer", PointSetSourceRenderer, SourceRenderer)

void PointSetSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, Source* source) const
{
	PointSetSource* pointSetSource = checked_type_cast< PointSetSource* >(source);

	resource::Proxy< PointSet > pointSet = pointSetSource->getPointSet();
	if (!pointSet.validate())
		return;

	const AlignedVector< PointSet::Point >& points = pointSet->getPoints();
	for (AlignedVector< PointSet::Point >::const_iterator i = points.begin(); i != points.end(); ++i)
		primitiveRenderer->drawSolidPoint(i->position, 3.0f, Color(255, 255, 0));
}

	}
}
