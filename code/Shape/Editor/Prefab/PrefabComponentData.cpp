/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Prefab/PrefabComponentData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.PrefabComponentData", 0, PrefabComponentData, world::IEntityComponentData)

int32_t PrefabComponentData::getOrdinal() const
{
	return 0;
}

void PrefabComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void PrefabComponentData::serialize(ISerializer& s)
{
}

}
