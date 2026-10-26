/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshModifierEntityComponentData.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.ai.NavMeshModifierEntityComponentData", 0, NavMeshModifierEntityComponentData, world::IEntityComponentData)

int32_t NavMeshModifierEntityComponentData::getOrdinal() const
{
	return 0;
}

void NavMeshModifierEntityComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void NavMeshModifierEntityComponentData::serialize(ISerializer& s)
{
}

}
