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

namespace traktor
{
	namespace mesh
	{

class SkinnedMesh;

	}

	namespace physics
	{

class PhysicsManager;

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

class EntityData;
class IEntityBuilder;

	}

	namespace animation
	{

class AnimatedMeshComponent;
class Skeleton;
class IPoseControllerData;

/*! Animated mesh entity data.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	struct Binding
	{
		std::wstring jointName;
		Ref< const world::EntityData > entityData;

		void serialize(ISerializer& s);
	};

	AnimatedMeshComponentData() = default;

	AnimatedMeshComponentData(
		const resource::Id< mesh::SkinnedMesh >& mesh,
		const resource::Id< Skeleton >& skeleton,
		const IPoseControllerData* poseController
	);

	Ref< AnimatedMeshComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< mesh::SkinnedMesh >& getMesh() const { return m_mesh; }

	const resource::Id< Skeleton >& getSkeleton() const { return m_skeleton; }

	const IPoseControllerData* getPoseControllerData() const { return m_poseController; }

	const AlignedVector< Binding >& getBindings() const { return m_bindings; }

private:
	resource::Id< mesh::SkinnedMesh > m_mesh;
	resource::Id< Skeleton > m_skeleton;
	Ref< const IPoseControllerData > m_poseController;
	bool m_screenSpaceCulling = true;
	AlignedVector< Binding > m_bindings;
};

	}
}
