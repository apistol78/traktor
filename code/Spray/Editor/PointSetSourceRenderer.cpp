/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/PointSet.h"
#include "Spray/Editor/PointSetSourceRenderer.h"
#include "Spray/Sources/PointSetSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSetSourceRenderer", PointSetSourceRenderer, SourceRenderer)

void PointSetSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const PointSetSourceData* pointSetSource = checked_type_cast< const PointSetSourceData* >(sourceData);

	//const resource::Proxy< PointSet >& pointSet = pointSetSource->getPointSet();
	//if (!pointSet)
	//	return;

	//const AlignedVector< PointSet::Point >& points = pointSet->get();
	//for (AlignedVector< PointSet::Point >::const_iterator i = points.begin(); i != points.end(); ++i)
	//	primitiveRenderer->drawSolidPoint(i->position + pointSetSource->getOffset(), 3.0f, Color4ub(255, 255, 0));
}

}
