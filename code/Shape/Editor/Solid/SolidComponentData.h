/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallSet.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class CollisionSpecification;

}

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class IEntityBuilder;

}

namespace traktor::shape
{

class PrimitiveEntityData;
class SolidComponent;

/*! Solid geometry component data.
 * \ingroup Shape
 *
 * Contain solid geometry primitives along with operation (intersection, union, difference etc).
 */
class T_DLLCLASS SolidComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	SolidComponentData();

	Ref< SolidComponent > createComponent(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	void setCollisionGroup(const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionGroup);

	const SmallSet< resource::Id< physics::CollisionSpecification > >& getCollisionGroup() const;

	void setCollisionMask(const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionMask);

	const SmallSet< resource::Id< physics::CollisionSpecification > >& getCollisionMask() const;

private:
	SmallSet< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
	SmallSet< resource::Id< physics::CollisionSpecification > > m_collisionMask;
};

}