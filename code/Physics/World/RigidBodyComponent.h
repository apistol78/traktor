/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityEvent;

}

namespace traktor::physics
{

class Body;
struct CollisionInfo;

/*!
 * \ingroup Physics
 */
class T_DLLCLASS RigidBodyComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit RigidBodyComponent(
		Body* body,
		world::IEntityEvent* eventCollide,
		float transformFilter
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	Body* getBody() const { return m_body; }

private:
	world::Entity* m_owner;
	Ref< Body > m_body;
	Ref< world::IEntityEvent > m_eventCollide;
	Transform m_lastTransform;
	float m_transformFilter;

	void collisionListener(const physics::CollisionInfo& collisionInfo);
};

}
