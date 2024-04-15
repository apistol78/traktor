/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Editor/PointSourceRenderer.h"
#include "Spray/Sources/PointSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSourceRenderer", PointSourceRenderer, SourceRenderer)

void PointSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const PointSourceData* pointSource = checked_type_cast< const PointSourceData* >(sourceData);
	primitiveRenderer->drawSolidPoint(pointSource->m_position, 8.0f, Color4ub(255, 255, 0, 180));
}

}
