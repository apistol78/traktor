/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
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

namespace traktor::world
{

class EntityData;
class IEntityBuilder;

}

namespace traktor::animation
{

class SkeletonComponent;
class Skeleton;
class IPoseControllerData;

/*! Skeleton component data.
 * \ingroup Animation
 */
class T_DLLCLASS SkeletonComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	struct Binding
	{
		std::wstring jointName;
		Ref< const world::EntityData > entityData;

		void serialize(ISerializer& s);
	};

	SkeletonComponentData() = default;

	SkeletonComponentData(
		const resource::Id< Skeleton >& skeleton,
		const IPoseControllerData* poseController
	);

	Ref< SkeletonComponent > createComponent(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Skeleton >& getSkeleton() const { return m_skeleton; }

	const IPoseControllerData* getPoseControllerData() const { return m_poseController; }

	const AlignedVector< Binding >& getBindings() const { return m_bindings; }

private:
	resource::Id< Skeleton > m_skeleton;
	Ref< const IPoseControllerData > m_poseController;
	AlignedVector< Binding > m_bindings;
};

}
