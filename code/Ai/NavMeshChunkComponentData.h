/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ai
{

class NavMeshChunk;

/*! Navigation mesh chunk component data.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshChunkComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	NavMeshChunkComponentData() = default;

	explicit NavMeshChunkComponentData(const resource::Id< NavMeshChunk >& chunk);

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< NavMeshChunk >& get() const { return m_chunk; }

private:
	resource::Id< NavMeshChunk > m_chunk;
};

}
