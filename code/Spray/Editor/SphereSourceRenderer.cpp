/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/PrimitiveRenderer.h"
#include "Spray/Editor/SphereSourceRenderer.h"
#include "Spray/Sources/SphereSourceData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SphereSourceRenderer", SphereSourceRenderer, SourceRenderer)

void SphereSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const SphereSourceData* sphereSource = checked_type_cast< const SphereSourceData* >(sourceData);

	const Vector4 position = sphereSource->m_position;
	const float minRadius = sphereSource->m_radius.min;
	const float maxRadius = sphereSource->m_radius.max;

	const Matrix44 T = translate(position);

	primitiveRenderer->drawWireSphere(T, minRadius, Color4ub(255, 255, 0));
	primitiveRenderer->drawWireSphere(T, maxRadius, Color4ub(255, 255, 0));
}

}
