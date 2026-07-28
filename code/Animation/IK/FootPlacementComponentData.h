/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/SmallSet.h"
#include "Physics/CollisionSpecification.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class PhysicsManager;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::animation
{

class FootPlacementComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS FootPlacementComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< FootPlacementComponent > createComponent(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager
	) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	SmallSet< resource::Id< physics::CollisionSpecification > > m_traceInclude;
	SmallSet< resource::Id< physics::CollisionSpecification > > m_traceIgnore;
	AlignedVector< std::wstring > m_footJoints;
	float m_offset = 0.1f;	//!< Foot joint offset from actual bottom of foot.
	float m_range = 0.2f;	//!< Trace range up/down from foot.
};

}
