/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/PrimitiveRenderer.h"
#include "Spray/Editor/BoxSourceRenderer.h"
#include "Spray/Sources/BoxSourceData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.BoxSourceRenderer", BoxSourceRenderer, SourceRenderer)

void BoxSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const BoxSourceData* boxSource = checked_type_cast< const BoxSourceData* >(sourceData);
	primitiveRenderer->drawWireAabb(boxSource->m_position, boxSource->m_extent, 1.0f, Color4ub(255, 255, 0));
}

}
