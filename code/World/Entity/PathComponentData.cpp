/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "World/Entity/PathComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.PathComponentData", 0, PathComponentData, IEntityComponentData)

PathComponentData::PathComponentData(const TransformPath& path)
:	m_path(path)
{
}

int32_t PathComponentData::getOrdinal() const
{
	return 0;
}

void PathComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void PathComponentData::serialize(ISerializer& s)
{
	s >> MemberComposite< TransformPath >(L"path", m_path);
}

}
