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

class NavMesh;

/*! Navigation mesh modifier component.
 * \ingroup AI
 *
 * Enable or disable navigation through owner entity's area.
 */
class T_DLLCLASS NavMeshModifierEntityComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	/*! Enable or disable navigation through area.
	 *
	 * \param enable True if area should be navigable.
	 */
	void setEnable(bool enable);

	/*! Check if navigation through area is enabled.
	 *
	 * \return True if area is navigable.
	 */
	bool isEnable() const { return m_enable; }

private:
	world::Entity* m_owner = nullptr;
	resource::Proxy< NavMesh > m_navMesh;
	bool m_enable = true;
};

}
