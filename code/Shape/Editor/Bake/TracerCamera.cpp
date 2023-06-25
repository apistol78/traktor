/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/TracerCamera.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerCamera", TracerCamera, Object)

TracerCamera::TracerCamera(
	const Transform& transform,
	float fieldOfView,
	int32_t width,
	int32_t height
)
:	m_transform(transform)
,	m_fieldOfView(fieldOfView)
,	m_width(width)
,	m_height(height)
{
}

	}
}