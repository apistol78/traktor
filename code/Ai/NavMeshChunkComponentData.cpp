/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshChunkComponentData.h"

#include "Ai/NavMeshChunk.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshChunkComponentData", 0, NavMeshChunkComponentData, world::IEntityComponentData)

NavMeshChunkComponentData::NavMeshChunkComponentData(const resource::Id< NavMeshChunk >& chunk)
	: m_chunk(chunk)
{
}

int32_t NavMeshChunkComponentData::getOrdinal() const
{
	return 0;
}

void NavMeshChunkComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void NavMeshChunkComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< NavMeshChunk >(L"chunk", m_chunk);
}

}
