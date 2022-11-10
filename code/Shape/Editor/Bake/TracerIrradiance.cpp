/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/TracerIrradiance.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerIrradiance", TracerIrradiance, Object)

TracerIrradiance::TracerIrradiance(
    db::Instance* irradianceInstance,
    const Aabb3& boundingBox
)
:   m_irradianceInstance(irradianceInstance)
,	m_boundingBox(boundingBox)
{
}

    }
}