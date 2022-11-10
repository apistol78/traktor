/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Resource/Id.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace physics
    {

class CollisionSpecification;

    }

	namespace render
	{

class IRenderSystem;

	}

    namespace resource
    {

class IResourceManager;

    }

    namespace world
    {

class IEntityBuilder;

    }

    namespace shape
    {

class PrimitiveEntityData;
class SolidEntity;

/*! Solid geometry entity data.
 * \ingroup Shape
 *
 * Contain solid geometry primitives along with operation (intersection, union, difference etc).
 */
class T_DLLCLASS SolidEntityData : public world::EntityData
{
    T_RTTI_CLASS;

public:
	SolidEntityData();

    Ref< SolidEntity > createEntity(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

    virtual void serialize(ISerializer& s) override final;

	void setCollisionGroup(const std::set< resource::Id< physics::CollisionSpecification > >& collisionGroup);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionGroup() const;

	void setCollisionMask(const std::set< resource::Id< physics::CollisionSpecification > >& collisionMask);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionMask() const;

private:
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionMask;
};

    }
}