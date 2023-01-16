/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Spline/SplineLayerComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineLayerComponentData", SplineLayerComponentData, world::IEntityComponentData)

int32_t SplineLayerComponentData::getOrdinal() const
{
	return 0;
}

void SplineLayerComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

	}
}
