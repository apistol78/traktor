/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Editor/EditorAttributesComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.EditorAttributesComponentData", 0, EditorAttributesComponentData, IEntityComponentData)

int32_t EditorAttributesComponentData::getOrdinal() const
{
	return 0;
}

void EditorAttributesComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void EditorAttributesComponentData::serialize(ISerializer& s)
{
	s >> Member< bool >(L"visible", visible);
	s >> Member< bool >(L"locked", locked);
	s >> Member< bool >(L"include", include);
	s >> Member< bool >(L"dynamic", dynamic);
}

}
