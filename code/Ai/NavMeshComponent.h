/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

class NavMesh;

/*! Navigation mesh entity.
 * \ingroup AI
 *
 * Navigation mesh entities are used to attach a navigation mesh
 * to a scene and thus being accessible through-out the engine.
 */
class T_DLLCLASS NavMeshComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	NavMeshComponent() = default;

	explicit NavMeshComponent(const resource::Proxy< NavMesh >& navMesh);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	/*! Get navigation mesh.
	 *
	 * \return Navigation mesh proxy.
	 */
	const resource::Proxy< NavMesh >& get() const { return m_navMesh; }

private:
	resource::Proxy< NavMesh > m_navMesh;
};

}
