/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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
class NavMeshComponent;

/*! Navigation mesh chunk component.
 * \ingroup AI
 *
 * Adds a chunk to navigation mesh of world while in world.
 */
class T_DLLCLASS NavMeshChunkComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit NavMeshChunkComponent(const resource::Proxy< NavMeshChunk >& chunk);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

private:
	resource::Proxy< NavMeshChunk > m_chunk;
	Ref< NavMeshComponent > m_navMeshComponent;	//!< Navigation mesh component chunk has been added to.
};

}
