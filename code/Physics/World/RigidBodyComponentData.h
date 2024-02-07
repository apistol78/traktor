/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class Entity;
class IEntityBuilder;
class IEntityEventData;

}

namespace traktor::physics
{

class BodyDesc;
class PhysicsManager;
class RigidBodyComponent;

/*!
 * \ingroup Physics
 */
class T_DLLCLASS RigidBodyComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	RigidBodyComponentData() = default;

	explicit RigidBodyComponentData(BodyDesc* bodyDesc);

	explicit RigidBodyComponentData(BodyDesc* bodyDesc, world::IEntityEventData* eventCollide);

	Ref< RigidBodyComponent > createComponent(
		const world::IEntityBuilder* entityBuilder,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const BodyDesc* getBodyDesc() const { return m_bodyDesc; }

	const world::IEntityEventData* getEventCollide() const { return m_eventCollide; }

private:
	Ref< BodyDesc > m_bodyDesc;
	Ref< world::IEntityEventData > m_eventCollide;
	float m_transformFilter = 0.0f;
};

}
